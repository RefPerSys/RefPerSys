#include <iostream>            // for cout and cin

class Rps_Value_Data                      // begin declaration of the class
{
public:                      // begin public section
    Rps_Value_Data(int initialValue);       // constructor
    Rps_Value_Data(const Rps_Value_Data& copy_from); //copy constructor
    Rps_Value_Data& operator=(const Rps_Value_Data& copy_from); //copy assignment
    ~Rps_Value_Data();                    // destructor

    int GetRps_Value_Data() const;        // accessor function
    void SetRps_Value_Data(int value);      // accessor function
    void Use();
private:                      // begin private section
    int itsValue;                // member variable
    char * string;
};

// constructor of Rps_Value_Data,
Rps_Value_Data::Rps_Value_Data(int initialValue)
{
    itsValue = initialValue;
    string = new char[10]();
}

//copy constructor for making a new copy of a Rps_Value_Data
Rps_Value_Data::Rps_Value_Data(const Rps_Value_Data& copy_from) {
    itsValue = copy_from.itsValue;
    string = new char[10]();
    std::copy(copy_from.string+0, copy_from.string+10, string);
}

//copy assignment for assigning a value from one Cat to another
Rps_Value_Data& Rps_Value_Data::operator=(const Rps_Value_Data& copy_from) {
    itsValue = copy_from.itsValue;
    std::copy(copy_from.string+0, copy_from.string+10, string);
}

// destructor, just an example
Rps_Value_Data::~Rps_Value_Data()
{
    delete[] string;
}

// GetValue, Public accessor function
// returns value of itsAge member
int Rps_Value_Data::GetRps_Value_Data() const
{
    return itsValue;
}

// Definition of SetValue, public
// accessor function
void Rps_Value_Data::SetRps_Value_Data(int value)
{
    // set member variable its age to
    // value passed in by parameter value
    itsValue = value;
}

// definition of Use method
// returns: void
// parameters: None
// action: Prints "usage" to screen
void Rps_Value_Data::Use()
{
    std::cout << " usage.\n";
}

// create a value, set it, use it
int test()
{
    int value;
    std::cout<<"What is the value? ";
    std::cin>>value;
    Rps_Value_Data theValue(value);
    theValue.Use();
    std::cout << "the value is a value which is " ;
    std::cout << theValue.GetRps_Value_Data() << " in usage.\n";
    theValue.Use();
    value++;
    theValue.SetRps_Value_Data(value);
    std::cout << "Now this value has " ;
    std::cout << theValue.GetRps_Value_Data() << " as its value.\n";
    return 0;
}
