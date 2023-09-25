#include <opencv2/opencv.hpp>
#include "xlsxwriter.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Глобальные константы
const std::string kImagePath = "C:\\Users\\ShadowLancel\\Desktop\\Junior\\j1.jpg";
const double kMinAspectRatio = 0.5;
const int kMinContourArea = 100;

//Заголовок функции генерации отчёта
void generateReport(const std::vector<std::vector<cv::Point>>& detectedContours, const std::vector<std::vector<cv::Point>>& groundTruthContours);

// Функция для загрузки изображения
cv::Mat loadImage(const std::string& imagePath) {
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Не удалось загрузить изображение." << std::endl;
        exit(-1);
    }
    return image;
}

// Функция для загрузки идеальных контуров
void loadGroundTruthContours(const std::string& filePath, std::vector<std::vector<cv::Point>>& groundTruthContours) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Не удалось открыть файл с эталонными контурами." << std::endl;
        exit(-1);
    }

    json data;
    inputFile >> data;

    if (data.contains("objects")) {
        const json& objects = data["objects"];

        for (const auto& obj : objects) {
            if (obj.contains("points") && obj["points"].contains("exterior")) {
                const json& points = obj["points"]["exterior"];
                std::vector<cv::Point> contour;

                for (const auto& point : points) {
                    int x = point[0];
                    int y = point[1];
                    contour.push_back(cv::Point(x, y));
                }

                groundTruthContours.push_back(contour);
            }
        }
    }

    inputFile.close();
}

// Функция для предварительной обработки изображения
cv::Mat preprocessImage(const cv::Mat& inputImage) {
    cv::Mat grayImage;
    cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);

    cv::Mat blurredImage;
    cv::GaussianBlur(grayImage, blurredImage, cv::Size(5, 5), 0);

    cv::Mat binaryImage;
    cv::threshold(blurredImage, binaryImage, 128, 255, cv::THRESH_BINARY);

    return binaryImage;
}

// Функция для фильтрации контуров по форме
std::vector<std::vector<cv::Point>> filterContoursByShape(const std::vector<std::vector<cv::Point>>& inputContours) {
    std::vector<std::vector<cv::Point>> filteredContours;

    for (const auto& contour : inputContours) {
        cv::Rect boundingRect = cv::boundingRect(contour);
        double aspectRatio = static_cast<double>(boundingRect.width) / boundingRect.height;

        if (aspectRatio > kMinAspectRatio) {
            filteredContours.push_back(contour);
        }
    }

    return filteredContours;
}

// Функция для поиска и фильтрации контуров
std::vector<std::vector<cv::Point>> findAndFilterContours(const cv::Mat& binaryImage) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> filteredContours;
    int minContourArea = kMinContourArea; // Минимальная площадь контура для учета

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area > minContourArea) {
            filteredContours.push_back(contour);
        }
    }

    // Фильтрация по форме
    return filterContoursByShape(filteredContours);
}

// Функция для создания изображения с рамками
cv::Mat createResultImage(const cv::Mat& inputImage, const std::vector<std::vector<cv::Point>>& contours) {
    cv::Mat resultImage = inputImage.clone();

    for (const auto& contour : contours) {
        cv::Rect boundingRect = cv::boundingRect(contour);
        cv::rectangle(resultImage, boundingRect, cv::Scalar(0, 0, 255), 2);
    }

    return resultImage;
}

int main() {
    // Загрузка изображения
    cv::Mat image = loadImage(kImagePath);

    // Загрузка эталонных контуров из JSON-файла
    std::vector<std::vector<cv::Point>> groundTruthContours;
    loadGroundTruthContours("j1.jpg.json", groundTruthContours);

    // Предварительная обработка изображения
    cv::Mat binaryImage = preprocessImage(image);

    // Поиск, фильтрация по площади и форме контуров
    std::vector<std::vector<cv::Point>> contours = findAndFilterContours(binaryImage);

    // Создание изображения с рамками
    cv::Mat resultImage = createResultImage(image, contours);

    // Отображение контуров
    cv::Mat contourImage = image.clone();
    cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2);

    // Сохранение результата
    cv::imwrite("result.jpg", resultImage);

    // Генерация отчета
    generateReport(contours, groundTruthContours);

    return 0;
}