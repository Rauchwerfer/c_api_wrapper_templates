#ifndef C_API_WRAPPER_TEMPLATES_H
#define C_API_WRAPPER_TEMPLATES_H

#if defined(_MSVC_LANG)
    #define C_WRAPPER_CPP_VERSION _MSVC_LANG
#else
    #define C_WRAPPER_CPP_VERSION __cplusplus
#endif

// #if C_WRAPPER_CPP_VERSION >= 201703L
//     #define C_WRAPPER_NODISCARD [[nodiscard]]
// #else
//     #define C_WRAPPER_NODISCARD
// #endif
#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(nodiscard) >= 201603
#    define C_WRAPPER_NODISCARD [[nodiscard]]
#  else
#    define C_WRAPPER_NODISCARD
#  endif
#else
#  define C_WRAPPER_NODISCARD
#endif


#ifdef __cplusplus
extern "C++" {

template<typename InstanceT>
class c_api_wrapper_view_class
{
protected:
    InstanceT* m_instance { nullptr };

public:
    c_api_wrapper_view_class() = default;
    explicit c_api_wrapper_view_class(InstanceT* instance) : m_instance(instance) {}

    C_WRAPPER_NODISCARD InstanceT* get_ptr() const noexcept { return m_instance; }
    C_WRAPPER_NODISCARD bool is_valid() const noexcept { return m_instance != nullptr; }
    C_WRAPPER_NODISCARD bool is_null() const noexcept { return m_instance == nullptr; }

    explicit operator bool() const noexcept { return is_valid(); }
};

template<
    //typename ViewT,
    typename InstanceT,
    void (*DestroyFn)(InstanceT*)
>
class c_api_wrapper_handle_class : public c_api_wrapper_view_class<InstanceT>//ViewT
{
public:
    c_api_wrapper_handle_class() = default;

    explicit c_api_wrapper_handle_class(InstanceT* instance)
        : c_api_wrapper_view_class<InstanceT>(instance)
    {
    }

    ~c_api_wrapper_handle_class() noexcept
    {
        reset();
    }

    c_api_wrapper_handle_class(const c_api_wrapper_handle_class&) = delete;
    c_api_wrapper_handle_class& operator=(const c_api_wrapper_handle_class&) = delete;

    c_api_wrapper_handle_class(c_api_wrapper_handle_class&& other) noexcept
        : c_api_wrapper_view_class<InstanceT>(other.m_instance)
    {
        other.m_instance = nullptr;
    }

    c_api_wrapper_handle_class& operator=(c_api_wrapper_handle_class&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            this->m_instance = other.m_instance;
            other.m_instance = nullptr;
        }

        return *this;
    }

    C_WRAPPER_NODISCARD InstanceT* release() noexcept
    {
        InstanceT* result = this->m_instance;
        this->m_instance = nullptr;
        return result;
    }

    void reset(InstanceT* instance = nullptr) noexcept
    {
        if (this->m_instance)
            DestroyFn(this->m_instance);

        this->m_instance = instance;
    }
};

template<
    //typename ViewT,
    typename InstanceT,
    InstanceT* (*CreateFn)(),
    void (*DestroyFn)(InstanceT*)
>
class c_api_wrapper_owner_class : public c_api_wrapper_handle_class<c_api_wrapper_view_class<InstanceT>, InstanceT, DestroyFn> //<ViewT, InstanceT, DestroyFn>
{
    using Base = c_api_wrapper_handle_class<c_api_wrapper_view_class<InstanceT>, InstanceT, DestroyFn>;

public:
    c_api_wrapper_owner_class()
        : Base(CreateFn())
    {
    }
};

} // extern "C++"
#endif // __cplusplus

#endif // C_API_WRAPPER_TEMPLATES_H