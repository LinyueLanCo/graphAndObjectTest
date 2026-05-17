//#include<iostream>
//#include<conio.h>
//#include<cstdlib>
//#include<graphics.h>
//#include"char.h"
//
//using namespace std;
//
//int main()
//{
//	Character hero1("Hero", 100, 20);
//	Character villain1("Monster", 80, 15);
//	cout << hero1.getName() << endl;
//	cout << villain1.getName() << endl;
//
//
//	while(hero1.getHealth() > 0 && villain1.getHealth() > 0)
//	{
//		hero1.attack(villain1);
//		hero1.displayStatus();
//		villain1.displayStatus();
//		if(villain1.getHealth() <= 0)
//		{
//			cout << villain1.getName() << " is defeated!" << endl;
//			break;
//		}
//
//		villain1.attack(hero1);
//		hero1.displayStatus();
//		villain1.displayStatus();
//		if(hero1.getHealth() <= 0)
//		{
//			cout << hero1.getName() << " is defeated!" << endl;
//			break;
//		}
//		_getch();
//
//	}
//	return 0;
//
//}
////作业1：简述什么是面向对象的程序设计，什么是面向过程的程序设计
////面向过程着重过程，也就是程序先干什么然后干什么最后干什么，面向对象则是将程序拆分为这个事件应该由谁干，让对象自己去处理干什么
//
////作业2：简述类和对象的关系
////类像是一个模板，可以用来创建多个具有相同属性和行为的对象，而对象则是一个使用类模板的一个实例