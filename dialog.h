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

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_lineEdit_editingFinished();

private:
    Ui::Dialog *ui;
    MyGraphicsScene * scene;
};
#endif // DIALOG_H
