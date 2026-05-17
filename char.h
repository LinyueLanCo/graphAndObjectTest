//#include<iostream>
//#include<string>
//using namespace std;
//class Character
//{
//private:
//    string name;
//    int health;
//    int attackPower;
//
//public:
//    Character(string n, int h, int a) : name(n), health(h), attackPower(a) {}
//
//    void takeDamage(int damage)
//    {
//        health -= damage;
//
//        if (health < 0)
//        {
//            health = 0;
//        }
//    }
//
//    void attack(Character& target)
//    {
//        target.takeDamage(attackPower);
//
//        cout << name << " attacks " << target.name
//            << " for " << attackPower << " damage!" << endl;
//    }
//
//    void displayStatus()
//    {
//        cout << name << " - Health: " << health
//            << ", Attack Power: " << attackPower << endl;
//    }
//
//    int getHealth()
//    {
//        return health;
//    }
//
//    string getName()
//    {
//        return name;
//    }
//};