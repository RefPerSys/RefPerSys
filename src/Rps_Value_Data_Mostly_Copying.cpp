#include <iostream>            // for cout and cin
#include "Rps_Value_Data.cpp"
class Rps_Value_Data_Mostly_Copying                      // begin declaration of the class
{
public:                      // begin public section
    Rps_Value_Data_Mostly_Copying(int initialValue);       // constructor
    Rps_Value_Data_Mostly_Copying(const Rps_Value_Data& copy_from); //copy constructor
    Rps_Value_Data_Mostly_Copying& operator=(const Rps_Value_Data& copy_from); //copy assignment
    ~Rps_Value_Data_Mostly_Copying();                    // destructor

    int GetRps_Value_Data_Mostly_Copying() const;        // accessor function
    void SetRps_Value_Data_Mostly_Copying(int value);      // accessor function
    void Use();
private:                      // begin private section
    int itsValue;                // member variable
    char * string;
};

// constructor of Rps_Value_Data,
Rps_Value_Data_Mostly_Copying::Rps_Value_Data_Mostly_Copying(int initialValue)
{
    itsValue = initialValue;
    string = new char[10]();
}

//copy constructor for making a new copy of a Rps_Value_Data_Mostly_Copying
Rps_Value_Data_Mostly_Copying::Rps_Value_Data_Mostly_Copying(const Rps_Value_Data& copy_from) {
    itsValue = copy_from.GetRps_Value_Data();
    string = new char[10]();
    //std::copy(copy_from.string+0, copy_from.string+10, string);
}

//copy assignment for assigning a value from one Rps_Value to another
Rps_Value_Data_Mostly_Copying& Rps_Value_Data_Mostly_Copying::operator=(const Rps_Value_Data& copy_from) {
    itsValue = copy_from.GetRps_Value_Data();
    //std::copy(copy_from.string+0, copy_from.string+10, string);
}

// destructor, just an example
Rps_Value_Data_Mostly_Copying::~Rps_Value_Data_Mostly_Copying()
{
    delete[] string;
}

// Get, Public accessor function
// returns value of itsAge member
int Rps_Value_Data_Mostly_Copying::GetRps_Value_Data_Mostly_Copying() const
{
    return itsValue;
}

// Definition of SetValue, public
// accessor function
void Rps_Value_Data_Mostly_Copying::SetRps_Value_Data_Mostly_Copying(int value)
{
    // set member variable its age to
    // value passed in by parameter value
    itsValue = value;
}

// definition of Use method
// returns: void
// parameters: None
// action: Prints "usage" to screen
void Rps_Value_Data_Mostly_Copying::Use()
{
    std::cout << " usage.\n";
}

// create a value, set it, use it
int test()
{
    int value;
    std::cout<<"What is the value? ";
    std::cin>>value;
    Rps_Value_Data_Mostly_Copying mostly_copying(value);
    mostly_copying.Use();
    std::cout << "the value is a value which is " ;
    std::cout << mostly_copying.GetRps_Value_Data_Mostly_Copying() << " in usage.\n";
    mostly_copying.Use();
    value++;
    mostly_copying.SetRps_Value_Data(value);
    std::cout << "Now this value has " ;
    std::cout << mostly_copying.GetRps_Value_Data() << " as its value.\n";
    return 0;
}
