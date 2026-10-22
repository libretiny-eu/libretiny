/**
 * @brief 
 * 
 * @tparam T 
 * @tparam i 
 */
template<class T, int i> class MyStack
{
    T*  pStack;
    T StackBuffer[i];
    static const int cItems = i * sizeof(T);
public:
    MyStack( void );
    void push( const T item );
    T& pop( void );
};


/**
 * @brief Construct a new My Stack<  T, i >:: My Stack object
 * 
 * @tparam T 
 * @tparam i 
 */
template< class T, int i > MyStack< T, i >::MyStack( void )
{
};
 
/**
 * @brief 
 * 
 * @tparam T 
 * @tparam i 
 * @param item 
 */
template< class T, int i > void MyStack< T, i >::push( const T item )
{
};

/**
 * @brief pop met
 * 
 * @tparam T myClass
 * @tparam i number
 * @return T& refer
 */
template< class T, int i > T& MyStack< T, i >::pop( void )
{
};

int main()
{
}