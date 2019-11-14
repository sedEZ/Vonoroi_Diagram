#ifndef DIALOG_H
#define DIALOG_H

#include "mygraphicsscene.h"
#include "wingededge.h"

#include <QMainWindow>
#include <QGraphicsView>
#include <QDialog>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QPointF>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void set_a_voronoi_from_tx();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_lineEdit_editingFinished();

private:
    Ui::Dialog *ui;
    MyGraphicsScene * scene;
    QTextStream *tx;
    QFile* file;
    bool from_tx;
};
#endif // DIALOG_H
