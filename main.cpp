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
 * Todo:11/08
 *  已經完成當按下Run的時候開始建立vonoroi 物件
 *  GraphicScene接下來需建立自己的RunOneStep去呼叫Vonoroi物件的RunOneStep
 *  並把回傳結果畫出,更新畫布
*/
