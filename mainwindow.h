#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <boost/filesystem.hpp>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <UsefulFunctions.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_histogram.h>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_pushButton_clicked();
    void on_pushButton_Analize_clicked();
    void on_pushButton_graphg2_clicked();
    void on_pushButton_g2far_clicked();
    void on_pushButton_plot_lifetime_clicked();
    void on_pushButton_plot_TimeTrace_clicked();
    boost::filesystem::path Append_to_name(boost::filesystem::path P, std::string string);
    void on_pushButton_plot_FreqHist_clicked();
private:
    Ui::MainWindow *ui;
    QwtPlotCurve *curveg2, *curveg2far, *curvelifetime, *curveTimeTrace;
    QwtPlotCurve *curveHistFreq;
//    QwtPlotHistogram *curveHistFreq;
};



#endif // MAINWINDOW_H
