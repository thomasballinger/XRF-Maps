/***

Copyright (c) 2016 Arthur Glowacki

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.

***/

#include "grapher.h"
#include <algorithm>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QValueAxis>
#include <QApplication>


namespace visual
{

void SavePlotSpectras(std::string path, data_struct::ArrayXr *energy, data_struct::ArrayXr *spectra, data_struct::ArrayXr *model, data_struct::ArrayXr *background, bool log_them)
{
    int argc = 0;
    char ** argv = nullptr;
    QApplication app(argc, argv);

    QtCharts::QLogValueAxis *axisYLog10 = new QtCharts::QLogValueAxis();
    axisYLog10->setTitleText("Counts Log10");
    axisYLog10->setLabelFormat("%.1e");
    //axisYLog10->setRange(1.0, 10000.0);
    axisYLog10->setBase(10.0);

    QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
    axisX->setTitleText("Energy (keV)");
    axisX->setLabelFormat("%1.0f");
    //axisX->setTickCount(series->count());
    //axisX->setRange(0, 2048);
    axisX->setTickCount(11);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("Counts");
    axisY->setLabelFormat("%i");
    //axisY->setTickCount(series->count());

    QtCharts::QChartView *chartView = new QtCharts::QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(1024, 768);

    QtCharts::QChart *chart = chartView->chart();
    chart->addAxis(axisX, Qt::AlignBottom);

    if(log_them)
    {
        chart->addAxis(axisYLog10, Qt::AlignLeft);
    }
    else
    {
        chart->addAxis(axisY, Qt::AlignLeft);
    }

    QtCharts::QLineSeries *series_spectra = new QtCharts::QLineSeries();
    QtCharts::QLineSeries *series_model = new QtCharts::QLineSeries();
    QtCharts::QLineSeries *series_background = new QtCharts::QLineSeries();

    series_spectra->setName("Integrated Spectra");
    series_spectra->setColor(QColor(Qt::green));
    series_model->setName("Model Spectra");
    series_model->setColor(QColor(Qt::red));
    series_background->setName("Background");
    series_background->setColor(QColor(Qt::blue));

    for(unsigned int i =0; i < spectra->rows(); i++)
    {

        float val_spec = (*spectra)[i];
        float val_mod = (*model)[i];
        float val_back = (*background)[i];

        bool isFine = std::isfinite(val_spec);
        if (false == isFine || val_spec <= 0.0f)
        {
            val_spec = 1.0;
        }
        isFine = std::isfinite(val_mod);
        if (false == isFine || val_mod <= 0.0f)
        {
            val_mod = 1.0;
        }
        isFine = std::isfinite(val_back);
        if (false == isFine || val_back <= 0.0f)
        {
            val_back = 1.0;
        }

        if(energy !=nullptr)
        {
            series_spectra->append((*energy)[i], val_spec);
            series_model->append((*energy)[i], val_mod);
            series_background->append((*energy)[i], val_back);
        }
        else
        {
            series_spectra->append(i, val_spec);
            series_model->append(i, val_mod);
            series_background->append(i, val_back);
        }
    }

    chart->addSeries(series_spectra);
    chart->addSeries(series_model);
    chart->addSeries(series_background);

    series_spectra->attachAxis(axisX);
    series_model->attachAxis(axisX);
    series_background->attachAxis(axisX);


    if(log_them)
    {
        series_spectra->attachAxis(axisYLog10);
        series_model->attachAxis(axisYLog10);
        series_background->attachAxis(axisYLog10);
    }
    else
    {
        series_spectra->attachAxis(axisY);
        series_model->attachAxis(axisY);
        series_background->attachAxis(axisY);
    }

    QPixmap pix = chartView->grab();
    QPainter painter(&pix);
    int h = 768;
    int w = 1024;
    int x = 0;
    int y = 0;
    painter.drawPixmap(x, y, w, h, pix);

    painter.end();
    pix.save(QString(path.c_str()), "png");

}

// ----------------------------------------------------------------------------

void SavePlotQuantification(std::string path, data_struct::Quantification_Standard *standard, int detector_num, int zstart, int zstop)
{
    for(auto& itr1 : standard->calibration_curves)
    {
        for(auto& itr2 : itr1.second.calib_curves)
        {
            std::string str_path_full = path + "calib_"+itr1.first+"_"+itr2.quantifier_name+"_det"+std::to_string(detector_num)+".png";
            SavePlotCalibrationCurve(str_path_full, &itr2, standard->element_weights(), zstart, zstop);
        }
    }
}

// ----------------------------------------------------------------------------

void SavePlotCalibrationCurve(std::string path, data_struct::Calibration_Curve *calib_curve, unordered_map<string, data_struct::Element_Quant> element_weights, int zstart, int zstop)
{
    //index starts at 0 so subtract one so we have correct z number
    zstart--;
    zstop--;
    int argc = 0;
    char ** argv = nullptr;
    QApplication app(argc, argv);

    QtCharts::QLogValueAxis *axisYLog10 = new QtCharts::QLogValueAxis();
    axisYLog10->setTitleText("Log10");
    axisYLog10->setLabelFormat("%.1e");
    axisYLog10->setBase(10.0);

    QtCharts::QCategoryAxis *axisX = new QtCharts::QCategoryAxis();
    axisX->setTitleText("Elements");
    QFont labelsFont;
    labelsFont.setPixelSize(12);
    axisX->setLabelsFont(labelsFont);
    axisX->setRange(zstart-1, zstop+1);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("Cts/ugcm2");

    QtCharts::QChartView *chartView = new QtCharts::QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(1024, 768);

    QtCharts::QChart *chart = chartView->chart();
    chart->addAxis(axisX, Qt::AlignBottom);
    //chart->addAxis(axisYLog10, Qt::AlignLeft);
    chart->addAxis(axisY, Qt::AlignLeft);


    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    series->setName(QString::fromStdString(calib_curve->quantifier_name));
    series->append(zstart-1, 0);
    for(int i=zstart; i <= zstop; i++)
    {
        axisX->append(QString::fromStdString(calib_curve->shell_curves_labels[0][i]), i);
        series->append(i, calib_curve->shell_curves[0][i]);
    }
    chart->addSeries(series);
    series->attachAxis(axisX);
    //series->attachAxis(axisYLog10);
    series->attachAxis(axisY);

    QtCharts::QScatterSeries *e_series = new QtCharts::QScatterSeries();
    e_series->setName(QString::fromStdString("axo"));
    for(auto& itr : element_weights)
    {
        data_struct::Element_Info* e_info = data_struct::Element_Info_Map::inst()->get_element(itr.first);
        if(e_info != nullptr)
        {
            e_series->append(e_info->number-1, itr.second.weight);
        }
    }
    chart->addSeries(e_series);
    e_series->attachAxis(axisX);
    //e_series->attachAxis(axisYLog10);
    e_series->attachAxis(axisY);



    QPixmap pix = chartView->grab();
    QPainter painter(&pix);
    int h = 768;
    int w = 1024;
    int x = 0;
    int y = 0;
    painter.drawPixmap(x, y, w, h, pix);

    painter.end();
    pix.save(QString(path.c_str()), "png");

}


} //namespace visual
