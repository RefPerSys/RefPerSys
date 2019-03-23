#include <iostream>            // for cout and cin

class Value                      // begin declaration of the class
{
public:                      // begin public section
    Value(int initialValue);       // constructor
    Value(const Value& copy_from); //copy constructor
    Value& operator=(const Value& copy_from); //copy assignment
    ~Value();                    // destructor

    int GetValue() const;        // accessor function
    void SetValue(int value);      // accessor function
    void Use();
private:                      // begin private section
    int itsValue;                // member variable
    char * string;
};

// constructor of Value,
Value::Value(int initialValue)
{
    itsValue = initialValue;
    string = new char[10]();
}

//copy constructor for making a new copy of a Value
Value::Value(const Value& copy_from) {
    itsValue = copy_from.itsValue;
    string = new char[10]();
    std::copy(copy_from.string+0, copy_from.string+10, string);
}

//copy assignment for assigning a value from one Cat to another
Value& Value::operator=(const Value& copy_from) {
    itsValue = copy_from.itsValue;
    std::copy(copy_from.string+0, copy_from.string+10, string);
}

// destructor, just an example
Value::~Value()
{
    delete[] string;
}

// GetValue, Public accessor function
// returns value of itsAge member
int Value::GetValue() const
{
    return itsValue;
}

// Definition of SetValue, public
// accessor function
void Value::SetValue(int value)
{
    // set member variable its age to
    // value passed in by parameter value
    itsValue = value;
}

// definition of Use method
// returns: void
// parameters: None
// action: Prints "usage" to screen
void Value::Use()
{
    std::cout << " usage.\n";
}

// create a value, set it, use it
int test()
{
    int value;
    std::cout<<"What is the value? ";
    std::cin>>value;
    Value theValue(value);
    theValue.Use();
    std::cout << "the value is a value which is " ;
    std::cout << theValue.GetValue() << " in usage.\n";
    theValue.Use();
    value++;
    theValue.SetValue(value);
    std::cout << "Now this value has " ;
    std::cout << theValue.GetValue() << " as its value.\n";
    return 0;
}
