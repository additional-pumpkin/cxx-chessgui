#include "mainwindow.hxx"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qlayoutitem.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <string>

#include "./ui_mainwindow.h"
#include "boardview.hxx"
#include "notationview.hxx"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    auto *parent_layout = new QSplitter(this);
    auto *board = new QWidget(this);
    auto *vlayout = new QVBoxLayout(this);
    auto *boardview = new BoardView(this);
    auto *notationview = new NotationView(this);
    auto *notation_scroll = new QScrollArea(this);
    auto *notation_vlayout = new QVBoxLayout(this);
    auto *fen_edit = new QLineEdit(this);
    auto *game_text = new QTextEdit(this);
    notation_scroll->setWidget(notationview);
    notation_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    notation_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    notation_scroll->setWidgetResizable(true);
    notation_scroll->setMinimumWidth(250);
    notation_vlayout->addWidget(notation_scroll);
    notation_vlayout->addWidget(game_text);
    auto *notation = new QWidget();
    notation->setLayout(notation_vlayout);
    ui->setupUi(this);
    vlayout->addWidget(boardview);
    vlayout->addWidget(fen_edit);
    boardview->setMinimumSize(600, 600);
    board->setLayout(vlayout);
    parent_layout->addWidget(board);
    parent_layout->setContentsMargins(0, 0, 0, 0);
    parent_layout->addWidget(notation);
    setCentralWidget(parent_layout);

    connect(fen_edit, &QLineEdit::editingFinished, boardview, [=]() { boardview->set_fen(fen_edit->text()); });
    connect(boardview, &BoardView::fen_changed, fen_edit,
            [=](const std::string &fen) { fen_edit->setText(QString::fromStdString(fen)); });
    connect(boardview, &BoardView::move_made, notationview, &NotationView::add_move);
    connect(boardview, &BoardView::forward, notationview, &NotationView::forward);
    connect(boardview, &BoardView::back, notationview, &NotationView::back);
    connect(notationview, &NotationView::back_move, boardview, &BoardView::back_move);
    connect(notationview, &NotationView::forward_move, boardview, &BoardView::forward_move);
    connect(notationview, &NotationView::move_added, boardview, &BoardView::move_added);
    connect(boardview, &BoardView::current_move, notationview, &NotationView::set_current_move);
    connect(boardview, &BoardView::get_prev_move, notationview, &NotationView::get_prev_move);
    connect(notationview, &NotationView::prev_move, boardview, &BoardView::set_prev_move);
    connect(notationview, &NotationView::text_changed, game_text,
            [=](const std::string &text) { game_text->setText(QString::fromStdString(text)); });
    boardview->set_fen(QStringLiteral("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/ w KQkq - 0 1"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
