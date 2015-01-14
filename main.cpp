#include "mainwindow.h"
#include <QApplication>
#include <QAction>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWindow;

    //Acceleration Keys:
    //Delete
    QAction* deleteAction = new QAction(&app);
    deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));
    mainWindow.addAction(deleteAction);
    mainWindow.connect(deleteAction, SIGNAL(triggered()), SLOT(doDelete()));

    //Select All
    QAction* selectAllAction = new QAction(&app);
    selectAllAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    mainWindow.addAction(selectAllAction);
    mainWindow.connect(selectAllAction, SIGNAL(triggered()), SLOT(doSelectAll()));

    //Copy
    QAction* copyAction = new QAction(&app);
    copyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    mainWindow.addAction(copyAction);
    mainWindow.connect(copyAction, SIGNAL(triggered()), SLOT(doCopy()));

    //Paste
    QAction* pasteAction = new QAction(&app);
    pasteAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
    mainWindow.addAction(pasteAction);
    mainWindow.connect(pasteAction, SIGNAL(triggered()), SLOT(doPaste()));

    //Open
    QAction* openAction = new QAction(&app);
    openAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    mainWindow.addAction(openAction);
    mainWindow.connect(openAction, SIGNAL(triggered()), SLOT(doImport()));

    //Save
    QAction* saveAction = new QAction(&app);
    saveAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    mainWindow.addAction(saveAction);
    mainWindow.connect(saveAction, SIGNAL(triggered()), SLOT(doSave()));

    //Save As(Export)
    QAction* exportAction = new QAction(&app);
    exportAction->setShortcut(QKeySequence(Qt::Key_F12));
    mainWindow.addAction(exportAction);
    mainWindow.connect(exportAction, SIGNAL(triggered()), SLOT(doExport()));

    //Undo
    QAction* undoAction = new QAction(&app);
    undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
    mainWindow.addAction(undoAction);
    mainWindow.connect(undoAction, SIGNAL(triggered()), SLOT(doUndo()));

    //Redo
    QAction* redoAction = new QAction(&app);
    redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
    mainWindow.addAction(redoAction);
    mainWindow.connect(redoAction, SIGNAL(triggered()), SLOT(doRedo()));

    //Print
    QAction* printAction = new QAction(&app);
    printAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    mainWindow.addAction(printAction);
    mainWindow.connect(printAction, SIGNAL(triggered()), SLOT(on_left_print_c_1_clicked()));

    //Execute!
    mainWindow.show();
    return app.exec();
}
