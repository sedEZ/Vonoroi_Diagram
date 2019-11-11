#include "dialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();
    return a.exec();
}


/*
 * Todo:
 *  11/08
 *  已經完成當按下Run的時候開始建立vonoroi 物件
 *  GraphicScene接下來需建立自己的RunOneStep去呼叫Vonoroi物件的RunOneStep
 *
 *  11/09:
 *  展開與合併應為兩個函式,目前的runOneStep是合併,但else的pseudo code寫成展開了
 *  constructTwoPointsVoronoi完成一半
 *  想法：新增兩個stack,一個放等待merge的,一個放已完成的
 *
 *
*/

