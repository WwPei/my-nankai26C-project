#include "emoji_dungeon_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EmojiDungeonWindow w;
    w.show();
    return a.exec();
}
