//#include <iostream>
//#include <string>
//using namespace std;
//
//class Cube
//{
//private:
//    int height;
//    int width;
//	int length;
//	int volume;
//public:
//	Cube(int h, int w, int l)
//	{
//		height = h;
//		width = w;
//		length = l;
//		volume = height * width * length;
//	}
//	int getVolume()
//	{
//        return volume;
//	}
//    int getheight()
//    {
//        return height;
//
//    }
//    int getwidth()
//    {
//        return width;
//    }
//    int getlength()
//    {
//        return length;
//
//    }
//
//
//};
//class Shape
//{
//public:
//    string type;
//
//    // 矩形：宽、高
//    int width;
//    int height;
//
//    // 圆形：半径
//    int radius;
//
//    // 三角形：三条边
//    int a;
//    int b;
//    int c;
//	//无参数构造函数，创建默认形状
//	Shape()
//	{
//		type = "Unknown";
//		width = 0;
//		height = 0;
//		radius = 0;
//		a = 0;
//		b = 0;
//		c = 0;
//	}
//	//两个参数构造函数，创建矩形
//    Shape(int w, int h)
//    {
//        type = "Rectangle";
//        width = w;
//        height = h;
//    }
//	//一个参数构造函数，创建圆形
//    Shape(int r)
//    {
//        type = "Circle";
//        radius = r;
//    }
//	//三个参数构造函数，创建三角形
//    Shape(int side1, int side2, int side3)
//    {
//        type = "Triangle";
//        a = side1;
//        b = side2;
//        c = side3;
//    }
//};
////二维地图类，动态开辟二维数组
//class Map
//{
//private:
//    int rows;
//    int cols;
//    int** data;
//
//public:
//    // 构造函数：创建地图
//    Map(int r, int c)
//    {
//        rows = r;
//        cols = c;
//
//        data = new int* [rows];
//
//        for (int i = 0; i < rows; i++)
//        {
//            data[i] = new int[cols];
//        }
//
//        // 初始化地图数据
//        for (int i = 0; i < rows; i++)
//        {
//            for (int j = 0; j < cols; j++)
//            {
//                data[i][j] = 0;
//            }
//        }
//    }
//
//    // 设置某个格子的值
//    void setGrid(int row, int col, int value)
//    {
//        if (row >= 0 && row < rows && col >= 0 && col < cols)
//        {
//            data[row][col] = value;
//        }
//    }
//
//    // 获取某个格子的值
//    int getGrid(int row, int col)
//    {
//        if (row >= 0 && row < rows && col >= 0 && col < cols)
//        {
//            return data[row][col];
//        }
//
//        return -1;
//    }
//
//    // 打印地图
//    void print()
//    {
//        for (int i = 0; i < rows; i++)
//        {
//            for (int j = 0; j < cols; j++)
//            {
//                cout << data[i][j] << " ";
//            }
//            cout << endl;
//        }
//    }
//
//    // 析构函数：释放地图
//    ~Map()
//    {
//        for (int i = 0; i < rows; i++)
//        {
//            delete[] data[i];
//        }
//
//        delete[] data;
//
//        data = nullptr;
//
//        cout << "releasedMemory" << endl;
//    }
//};
//int main()
//{
//	Shape defaultShape;
//	Shape rect(10, 5);
//	Shape circle(7);
//	Shape triangle(3, 4, 5);
//	cout << "Shape: " << defaultShape.type << endl;
//	cout << "Shape: " << rect.type << ", Width: " << rect.width << ", Height: " << rect.height << endl;
//	cout << "Shape: " << circle.type << ", Radius: " << circle.radius << endl;
//	cout << "Shape: " << triangle.type << ", Sides: " << triangle.a << ", " << triangle.b << ", " << triangle.c << endl;
//    {
//        Map gameMap(5, 5);
//        gameMap.setGrid(2, 2, 1);
//        gameMap.print();
//    }
//
//	Cube cube1(3, 4, 5);
//    Cube cube2(2, 3, 4);
//	cout << "Cube 1 - Height: " << cube1.getheight() << ", Width: " << cube1.getwidth() << ", Length: " << cube1.getlength() << ", Volume: " << cube1.getVolume() << endl;
//	cout << "Cube 2 - Height: " << cube2.getheight() << ", Width: " << cube2.getwidth() << ", Length: " << cube2.getlength() << ", Volume: " << cube2.getVolume() << endl;  
//	cout << "the total volume of the two cubes is " << cube1.getVolume() + cube2.getVolume() << endl;
//	return 0;
//}   