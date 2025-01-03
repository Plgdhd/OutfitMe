#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace cv;
using namespace dnn;
using namespace std;

// --- Функция отображения изображения ---
Mat overlayImage(const Mat& background, const Mat& foreground, Point2i location, Size itemSize) {
    if (background.empty() || foreground.empty()) {
        cerr << "[ERROR] Одно из изображений пустое!" << endl;
        return background.clone();
    }

    if (foreground.channels() != 4) {
        cerr << "[ERROR] Изображение одежды должно иметь 4 канала (RGBA)!" << endl;
        return background.clone();
    }

    Mat output = background.clone();
    Mat resizedItem;
    resize(foreground, resizedItem, itemSize);

    for (int y = 0; y < resizedItem.rows; ++y) {
        for (int x = 0; x < resizedItem.cols; ++x) {
            int bgY = location.y + y;
            int bgX = location.x + x;

            if (bgY >= 0 && bgY < background.rows && bgX >= 0 && bgX < background.cols) {
                Vec4b fgPixel = resizedItem.at<Vec4b>(y, x);
                Vec3b& bgPixel = output.at<Vec3b>(bgY, bgX);

                float alpha = fgPixel[3] / 255.0f;
                for (int c = 0; c < 3; ++c) {
                    bgPixel[c] = saturate_cast<uchar>(alpha * fgPixel[c] + (1.0f - alpha) * bgPixel[c]);
                }
            }
        }
    }

    return output;
}

// --- Функция обнаружения ключевых точек тела ---
vector<Point> detectBodyKeypoints(const Mat& person, const string& modelPath, const string& protoPath) {
    vector<Point> keypoints;

    Net net = readNet(modelPath, protoPath);
    if (net.empty()) {
        cerr << "[ERROR] Ошибка загрузки модели OpenPose!" << endl;
        return keypoints;
    }

    // Преобразование изображения в формат для модели
    Mat blob;
    blobFromImage(person, blob, 1.0 / 255.0, Size(368, 368), Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    Mat output = net.forward();

    int H = output.size[2]; // Высота карты
    int W = output.size[3]; // Ширина карты

    const int NUM_KEYPOINTS = 25;
    for (int i = 0; i < NUM_KEYPOINTS; ++i) {
        Mat heatMap(H, W, CV_32F, output.ptr(0, i));
        Point maxLoc;
        double maxVal;

        minMaxLoc(heatMap, 0, &maxVal, 0, &maxLoc);

        if (maxVal > 0.1) { // Уверенность > 0.1
            keypoints.push_back(Point(static_cast<int>(maxLoc.x * person.cols / W),
                static_cast<int>(maxLoc.y * person.rows / H)));
        }
        else {
            keypoints.push_back(Point(-1, -1));
        }
    }

    return keypoints;
}

// --- Функция вычисления положения и размера майки ---
template <typename T>
constexpr const T& clamp(const T& value, const T& low, const T& high) {
    return (value < low) ? low : (value > high) ? high : value;
}
Point calculateTshirtPosition(vector<Point>& keypoints, Size tshirtSize) {
    if (keypoints[1].x == -1 || keypoints[1].y == -1 ||
        keypoints[2].x == -1 || keypoints[5].x == -1) {
        cerr << "[ERROR] Точки шеи или плеч не обнаружены!" << endl;
        return Point(0, 0);
    }

    // Изменено: располагем майку выше и по центру между плечами
    //int x = keypoints[8].y - tshirtSize.width / 2.75; // Центр по тазу
    int x = ((keypoints[8].x + keypoints[16].x) / 2 - tshirtSize.width / 2); // Центр по тазу
    int y = keypoints[1].y - tshirtSize.height / 10; // Изменено: выше шеи
    return Point(x, y);
}

Size calculateTshirtSize(vector<Point>& keypoints, const Mat& tshirt) {
    if (keypoints[2].x == -1 || keypoints[5].x == -1 ||
        keypoints[8].x == -1 || keypoints[8].y == -1) {
        cerr << "[ERROR] Точки плеч или таза не обнаружены! Используется стандартный размер одежды." << endl;
        return Size(tshirt.cols, tshirt.rows);
    }

    // Изменено: увеличиваем ширину майки
    int bodyWidth = abs(keypoints[5].x - keypoints[2].x); // Расстояние между плечами
    int bodyHeight = abs(keypoints[8].y - keypoints[1].y); // Высота от шеи до таза

    if (bodyWidth <= 0 || bodyHeight <= 0) {
        cerr << "[ERROR] Некорректные размеры тела! Используется стандартный размер одежды." << endl;
        return Size(tshirt.cols, tshirt.rows);
    }

    // Изменено: Увеличил коэффициенты ширины и высоты
    float scaleFactorWidth = static_cast<float>(bodyWidth) / tshirt.cols * 2.2; // Изменено: шире на 100%
    float scaleFactorHeight = static_cast<float>(bodyHeight) / tshirt.rows * 1.2; // Высота увеличена на 20%

    int newWidth = static_cast<int>(tshirt.cols * scaleFactorWidth);
    int newHeight = static_cast<int>(tshirt.rows * scaleFactorHeight);

    // Граничные проверки (оставил как было)
    const int minSize = 50;
    const int maxSize = 1000;
    newWidth = clamp(newWidth, minSize, maxSize);
    newHeight = clamp(newHeight, minSize, maxSize);

    return Size(newWidth, newHeight);
}

// --- Функция вычисления положения и размера штанов ---
Point calculatePantsPosition(vector<Point>& keypoints, Size pantsSize) {
    if (keypoints[8].x == -1 || keypoints[8].y == -1 ||
        keypoints[9].x == -1 || keypoints[12].x == -1) {
        cerr << "[ERROR] Точки таза или бедер не обнаружены!" << endl;
        return Point(0, 0);
    }

    // Положение штанов: среднее между центром головы и таза
    int x = ((keypoints[8].x + keypoints[16].x) / 2 - pantsSize.width / 2);
    int y = keypoints[8].y * 0.98; // Штаны начинаются от таза
    return Point(x, y);
}

Size calculatePantsSize(vector<Point>& keypoints, const Mat& pants) {
    if (keypoints[9].x == -1 || keypoints[12].x == -1 ||
        keypoints[10].y == -1 || keypoints[13].y == -1) {
        cerr << "[ERROR] Точки бедер или коленей не обнаружены! Используется стандартный размер одежды." << endl;
        return Size(pants.cols, pants.rows);
    }

    // Ширина штанов: расстояние между бедрами
    int hipWidth = abs(keypoints[5].x - keypoints[2].x);
    // Высота штанов: расстояние от таза до коленей
    int pantsHeight = abs(keypoints[10].y - keypoints[24].y);

    if (hipWidth <= 0 || pantsHeight <= 0) {
        cerr << "[ERROR] Некорректные размеры тела! Используется стандартный размер одежды." << endl;
        return Size(pants.cols, pants.rows);
    }

    // Коэффициенты ширины и высоты для масштабирования штанов
    float scaleFactorWidth = static_cast<float>(hipWidth) / pants.cols * 1.6; // Увеличение на 130%
    float scaleFactorHeight = static_cast<float>(pantsHeight) / pants.rows * 2; // Увеличение на 130%

    int newWidth = static_cast<int>(pants.cols * scaleFactorWidth);
    int newHeight = static_cast<int>(pants.rows * scaleFactorHeight);

    // Граничные проверки
    const int minSize = 50;
    const int maxSize = 1000;
    newWidth = clamp(newWidth, minSize, maxSize);
    newHeight = clamp(newHeight, minSize, maxSize);

    return Size(newWidth, newHeight);
}

// --- Функция вычисления положения и размера шляпы ---
Point calculateHatPosition(vector<Point>& keypoints, Size hatSize) {
    if (keypoints[0].x == -1 || keypoints[0].y == -1) { // Точка головы
        cerr << "[ERROR] Точка головы не обнаружена!" << endl;
        return Point(0, 0);
    }

    int x = keypoints[16].x - hatSize.width / 2; // Центр по голове
    int y = keypoints[16].y - hatSize.height;   // Над головой
    return Point(x, y);
}

Size calculateHatSize(vector<Point>& keypoints, const Mat& hat) {
    if (keypoints[0].x == -1 || keypoints[0].y == -1 || keypoints[1].x == -1 || keypoints[1].y == -1) {
        cerr << "[ERROR] Точки головы не обнаружены! Используется стандартный размер шляпы." << endl;
        return Size(hat.cols, hat.rows);
    }

    int headWidth = abs(keypoints[16].x - keypoints[17].x) * 2.5; // Примерная ширина головы
    float scaleFactor = static_cast<float>(headWidth) / hat.cols;

    int newWidth = static_cast<int>(hat.cols * scaleFactor);
    int newHeight = static_cast<int>(hat.rows * scaleFactor);

    const int minSize = 50;
    const int maxSize = 500;
    newWidth = clamp(newWidth, minSize, maxSize);
    newHeight = clamp(newHeight, minSize, maxSize);

    return Size(newWidth, newHeight);
}

// --- Функция вычисления положения и размера очков ---
Point calculateGlassesPosition(vector<Point>& keypoints, Size glassesSize) {
    if (keypoints[1].x == -1 || keypoints[1].y == -1 || keypoints[2].x == -1 || keypoints[5].x == -1) {
        cerr << "[ERROR] Точки глаз или головы не обнаружены!" << endl;
        return Point(0, 0);
    }

    int x = (keypoints[0].x + keypoints[18].x) / 2 - glassesSize.width / 2; // Центр между глазами
    int y = keypoints[18].y - glassesSize.height / 3.5; // Чуть ниже верхней точки головы
    return Point(x, y);
}

Size calculateGlassesSize(vector<Point>& keypoints, const Mat& glasses) {
    if (keypoints[1].x == -1 || keypoints[2].x == -1 || keypoints[5].x == -1) {
        cerr << "[ERROR] Точки глаз не обнаружены! Используется стандартный размер очков." << endl;
        return Size(glasses.cols, glasses.rows);
    }

    int eyeDistance = abs(keypoints[18].x - keypoints[0].x); // Расстояние между глазами
    float scaleFactor = static_cast<float>(eyeDistance) / glasses.cols * 2.2;

    int newWidth = static_cast<int>(glasses.cols * scaleFactor);
    int newHeight = static_cast<int>(glasses.rows * scaleFactor);

    const int minSize = 30;
    const int maxSize = 300;
    newWidth = clamp(newWidth, minSize, maxSize);
    newHeight = clamp(newHeight, minSize, maxSize);

    return Size(newWidth, newHeight);
}

// --- Функция обработки запроса из Flutter ---
void processClothingRequest(const string& clothingType, const Mat& person, const string& modelPath, const string& protoPath) {
    //получение фото одежды
    string clothInput = "H:\\OutfitME\\outfit_me\\clTest\\x64\\Debug\\wearPath.txt";
    ifstream inputFile(clothInput);
    string buffer;
    if (!inputFile.is_open()) {
        cerr << "Не удалось открыть wearPath.txt!" << endl;
        return;
    }
    getline(inputFile, buffer);
    string clothPath = "H:/OutfitME/outfit_me/" + buffer; // Это значение должно быть передано из Flutter !!!!!!!!!!!!!!!!!!!!!!

    // Загружаем модель для ключевых точек
    vector<Point> keypoints = detectBodyKeypoints(person, modelPath, protoPath);
    if (keypoints.empty()) {
        cerr << "[ERROR] Не удалось обнаружить ключевые точки!" << endl;
        return;
    }

    Mat clothingItem, output;
    Size itemSize;
    Point itemLocation;

    // В зависимости от запроса выбираем, что накладывать
    if (clothingType == "tshirt") {
        clothingItem = imread(clothPath, IMREAD_UNCHANGED);
        itemSize = calculateTshirtSize(keypoints, clothingItem);
        itemLocation = calculateTshirtPosition(keypoints, itemSize);
    }
    else if (clothingType == "pants") {
        clothingItem = imread(clothPath, IMREAD_UNCHANGED);
        itemSize = calculatePantsSize(keypoints, clothingItem);
        itemLocation = calculatePantsPosition(keypoints, itemSize);
    }
    else if (clothingType == "hat") {
        clothingItem = imread(clothPath, IMREAD_UNCHANGED);
        itemSize = calculateHatSize(keypoints, clothingItem);
        itemLocation = calculateHatPosition(keypoints, itemSize);
    }
    else if (clothingType == "glasses") {
        clothingItem = imread(clothPath, IMREAD_UNCHANGED);
        itemSize = calculateGlassesSize(keypoints, clothingItem);
        itemLocation = calculateGlassesPosition(keypoints, itemSize);
    }
    else {
        cerr << "[ERROR] Неверный тип одежды!" << endl;
        return;
    }

    // Наложение выбранной одежды на изображение
    output = overlayImage(person, clothingItem, itemLocation, itemSize);

    // Сохранение и отображение результата
    imwrite("result_with_selected_item.jpg", output);
    //imshow("Result", output);
    waitKey(0);
}

// --- Главная функция (обновленная) ---

string readFileToString(const string& filePath) {
    ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        cerr << "[ERROR] Не удалось открыть файл: " << filePath << endl;
        return "";
    }
    string line;
    getline(inputFile, line);
    inputFile.close();
    return line;
}

int main() {
    setlocale(LC_ALL, "Russian");

    // Чтение пути к изображению
    string personInput = "H:\\OutfitME\\outfit_me\\clTest\\x64\\Debug\\input.txt";
    string personPath = readFileToString(personInput);
    if (personPath.empty()) {
        return -1;
    }

    // Загрузка изображения
    Mat person = imread(personPath);
    if (person.empty()) {
        cerr << "[ERROR] Не удалось загрузить изображение: " << personPath << endl;
        return -1;
    }

    // Чтение типа одежды
    string clothingTypePath = "H:\\OutfitME\\outfit_me\\clTest\\x64\\Debug\\wearType.txt";
    string clothingType = readFileToString(clothingTypePath);
    if (clothingType.empty()){
        return -1;
    }

    string modelPath = "H:/OutfitME/outfit_me/clTest/x64/Debug/pose_iter_584000.caffemodel";
    string protoPath = "H:/OutfitME/outfit_me/openpose/models/pose/body_25/pose_deploy.prototxt";

    processClothingRequest(clothingType, person, modelPath, protoPath);

    return 0;
}



/*#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
using namespace cv;
using namespace std;
using namespace samples;
int main() {

	CascadeClassifier faceCascade;
	if (!faceCascade.load(findFile("H:/OutfitME/outfit_me/clTest/x64/Debug/haarcascade_frontalface_default.xml"))) {
		cerr << "Ошибка: не удалось загрузить каскад!" << endl;
		return -1;
	}
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "Ошибка: не удалось открыть камеру!" << endl;
		return -1;
	}

	Mat frame;
	while (true) {
		cap.read(frame);
		if (frame.empty()) {
			cerr << "Ошибка: пустой кадр!" << endl;
			break;
		}
		flip(frame, frame, 1);
		Mat grayFrame;
		cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
		equalizeHist(grayFrame, grayFrame); 

		vector<Rect> faces;
		faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, Size(30, 30));

		for (const auto& face : faces) {
			rectangle(frame, face, Scalar(59, 89, 89), 2);
		}
		imshow("Face Tracking", frame);
		if (waitKey(30) == 'q') {
			break;
		}
	}

	return 0;
}

/*#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	VideoCapture cap(0);
	if (cap.isOpened() == false)
	{
		cout << "Cannot open the video camera" << endl;
		cin.get(); 
		return -1;
	}

	double dWidth = cap.get(CAP_PROP_FRAME_WIDTH); 
	double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT);

	cout << "Resolution of the video : " << dWidth << " x " << dHeight << endl;

	string window_name = "My Camera";
	namedWindow(window_name); 

	while (true)
	{
		Mat frame;
		bool bSuccess = cap.read(frame); 
		if (bSuccess == false)
		{
			cout << "Video camera is disconnected" << endl;
			cin.get();
			break;
		}
		Mat inverse;
		flip(frame, inverse, 1);
		Mat brighNessFrame;
		int brighNess = 10;
		frame.convertTo(brighNessFrame, -1, 1, brighNess);
		if (waitKey(10) == 49) {
			brighNess--;
			frame.convertTo(brighNessFrame, -1, 1, brighNess);
		}
		if (waitKey(10) == 50) {
			brighNess++;
			frame.convertTo(brighNessFrame, -1, 1, brighNess);
		}
		flip(frame, brighNessFrame, 1);
		imshow(window_name, brighNessFrame);

		if (waitKey(10) == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
	}

	return 0;
}*/