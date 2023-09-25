#include <opencv2/opencv.hpp>
#include "xlsxwriter.h"

void generateReport(const std::vector<std::vector<cv::Point>>& detectedContours, const std::vector<std::vector<cv::Point>>& groundTruthContours) {
    // Создание нового файла XLS
    lxw_workbook* workbook = workbook_new("report.xlsx");
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, NULL);

    // Названия столбцов
    worksheet_write_string(worksheet, 0, 0, "Image", NULL);
    worksheet_write_string(worksheet, 0, 1, "Detected Matches", NULL);
    worksheet_write_string(worksheet, 0, 2, "Ground Truth Matches", NULL);
    worksheet_write_string(worksheet, 0, 3, "Detection Percentage", NULL);
    worksheet_write_string(worksheet, 0, 4, "False Positives", NULL);

    // Запись данных в файл
    for (int i = 0; i < detectedContours.size(); i++) {
        std::string imageName = "Image_" + std::to_string(i);
        int detectedMatches = detectedContours[i].size();
        int bestGroundTruthMatches = 0;
        double bestDetectionPercentage = 0.0;
        int bestFalsePositives = 0;

        // Вычисляем координаты левого верхнего угла и правого нижнего угла для обнаруженного контура
        cv::Rect detectedRect = cv::boundingRect(detectedContours[i]);
        for (int j = 0; j < groundTruthContours.size(); j++) {
            int groundTruthMatches = groundTruthContours[j].size();

            // Вычисляем координаты левого верхнего угла и правого нижнего угла для истинного контура
            cv::Rect groundTruthRect = cv::boundingRect(groundTruthContours[j]);

            // Вычисляем процент совпадения на основе координат прямоугольников
            cv::Rect intersection = detectedRect & groundTruthRect;
            double detectionPercentage = (static_cast<double>(intersection.area()) / detectedRect.area()) * 100.0;
            int falsePositives = detectedMatches - groundTruthMatches;

            if (detectionPercentage > bestDetectionPercentage) {
                bestGroundTruthMatches = groundTruthMatches;
                bestDetectionPercentage = detectionPercentage;
                bestFalsePositives = falsePositives;
            }
        }

        // Записываем наилучшее совпадение в файл
        worksheet_write_string(worksheet, i + 1, 0, imageName.c_str(), NULL);
        worksheet_write_number(worksheet, i + 1, 1, detectedMatches, NULL);
        worksheet_write_number(worksheet, i + 1, 2, bestGroundTruthMatches, NULL);
        worksheet_write_number(worksheet, i + 1, 3, bestDetectionPercentage, NULL);
        worksheet_write_number(worksheet, i + 1, 4, bestFalsePositives, NULL);
    }

    workbook_close(workbook);
}