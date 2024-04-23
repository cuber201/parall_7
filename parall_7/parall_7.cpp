// parall_7.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <mpi.h>

using namespace cv;
using namespace std;

const int MAX_ITER = 1000;

int mandelbrotSet(double real, double imag) {
    double z_real = 0, z_imag = 0;
    int iter = 0;
    while (iter < MAX_ITER && (z_real * z_real + z_imag * z_imag) < 4.0) {
        double temp_real = z_real * z_real - z_imag * z_imag + real;
        z_imag = 2 * z_real * z_imag + imag;
        z_real = temp_real;
        iter++;
    }
    return iter;
}


Vec3b iterToColor(int iter) {
    if (iter == MAX_ITER) {
        return Vec3b(0, 0, 0);
    }
    double hue = static_cast<double>(iter) / MAX_ITER * 360.0;
    return Vec3b(hue / 2.0 * 255, hue  / 3.0 * 255, hue / 5.5 * 255);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int WIDTH = 800;
    const int HEIGHT = 800;
    const double X_MIN = -2.0;
    const double X_MAX = 1.0;
    const double Y_MIN = -1.5;
    const double Y_MAX = 1.5;
    const double X_STEP = (X_MAX - X_MIN) / WIDTH;
    const double Y_STEP = (Y_MAX - Y_MIN) / HEIGHT;

    int rows_per_process = HEIGHT / size;
    int start_row = rank * rows_per_process;
    int end_row = (rank == size - 1) ? HEIGHT : start_row + rows_per_process;

    int* mandelbrot_data = new int[WIDTH * (end_row - start_row)];

    for (int j = start_row; j < end_row; j++) {
        for (int i = 0; i < WIDTH; i++) {
            double real = X_MIN + i * X_STEP;
            double imag = Y_MIN + j * Y_STEP;
            mandelbrot_data[(j - start_row) * WIDTH + i] = mandelbrotSet(real, imag);
        }
    }

    int* all_data = nullptr;
    if (rank == 0) {
        all_data = new int[WIDTH * HEIGHT];
    }
    MPI_Gather(mandelbrot_data, WIDTH * rows_per_process, MPI_INT,
        all_data, WIDTH * rows_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        Mat mandelbrot_image(HEIGHT, WIDTH, CV_8UC3);
        for (int j = 0; j < HEIGHT; j++) {
            for (int i = 0; i < WIDTH; i++) {
                int iter = all_data[j * WIDTH + i];
                cv::Vec3b color = iterToColor(iter);
                mandelbrot_image.at<cv::Vec3b>(j, i) = color;
            }
        }
        cvtColor(mandelbrot_image, mandelbrot_image, COLOR_HSV2BGR);
        imshow("Mandelbrot Set", mandelbrot_image);
        waitKey(0);
    }

    delete[] mandelbrot_data;
    if (rank == 0) {
        delete[] all_data;
    }

    MPI_Finalize();
    return 0;
}



// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
