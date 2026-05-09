#include <QApplication>

#include "emoji_dungeon_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Emoji Dungeon"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QApplication::setOrganizationName(QStringLiteral("EmojiDungeon"));

    EmojiDungeonWindow window;
    window.show();

    return app.exec();
}
