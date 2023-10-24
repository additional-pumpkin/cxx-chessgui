#include "notationview.hxx"

#include <QFontDatabase>
#include <QPainter>
#include <cstdint>
#include <qnamespace.h>
NotationView::NotationView(QWidget *parent)
    : QWidget{parent}
    , m_move_height(30)
    , m_current_move_id(0)
    , m_current_move_index(0)
{
    int success = QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoChess.ttf"));
    QFontDatabase::applicationFontFamilies(0);
    setFocusPolicy(Qt::StrongFocus);

    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);
}

void NotationView::back()
{
    db::Move move = m_game.back();
    emit back_move(m_game.current_move(), move);
    emit text_changed(m_game.text());
    // set_current_move(m_game.current_move());
    update();
}

void NotationView::forward()
{
    db::Move move = m_game.forward();
    emit forward_move(m_game.current_move(), move);
    emit text_changed(m_game.text());
    // set_current_move(m_game.current_move());
    update();
}

void NotationView::add_move(const db::Move &move, bool animated)
{
    m_game.add_move(move);
    emit text_changed(m_game.text());
    if (height() < move.full_move * 30)
        setMinimumSize(QSize(width(), move.full_move * 30));
    if (!animated)
        emit move_added(m_game.current_move(), move, false);
    else
        emit move_added(m_game.current_move(), move);
    update();
}

void NotationView::set_current_move(const db::MoveId id)
{
    m_current_move_id = id;
    update();
}

void NotationView::recalculate_moveboxes()
{
    m_move_boxes.clear();
    auto move_nodes = m_game.get_moves();
    m_move_boxes.reserve(move_nodes.size());
    const int move_box_width = 100;
    const int move_box_height = m_move_height;
    const int move_number_width = 40;
    for (auto &move_node : move_nodes) {
        if (move_node.variation_level > 0)
            continue;
        MoveBox move_box;
        QPointF pos(move_node.move.color == db::BLACK ? move_box_width + move_number_width : move_number_width,
                    (move_node.move.full_move - 1) * move_box_height);
        move_box.node = move_node;
        move_box.hitbox = QRectF(pos, QSizeF(move_box_width, m_move_height));
        m_move_boxes.push_back(move_box);
    }
}

void NotationView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QColor(77, 77, 77));
    painter.setBrush(QColor(247, 246, 245));
    QFont font(QStringLiteral("Noto Chess"));
    font.setPointSize(12);
    painter.setFont(font);

    recalculate_moveboxes();
    for (MoveBox move_box : m_move_boxes) {
        if (move_box.node.move.color == db::WHITE) {
            painter.setPen(Qt::transparent);
            painter.drawRect(
                QRectF(QPointF(0, (move_box.node.move.full_move - 1) * m_move_height), QSizeF(40, m_move_height)));
            painter.setPen(QColor(179, 179, 179));
            painter.drawText(
                QRectF(QPointF(0, (move_box.node.move.full_move - 1) * m_move_height), QSizeF(40, m_move_height)),
                Qt::AlignCenter, QString::number(move_box.node.move.full_move));
            painter.setPen(QColor(77, 77, 77));
        }
        if (move_box.node.move_id == m_current_move_id) {
            painter.fillRect(move_box.hitbox, QColor(198, 221, 243));
            font.setBold(true);
            painter.setFont(font);
            painter.setPen(QColor(31, 31, 31));

        } else {
            painter.fillRect(move_box.hitbox, Qt::white);
        }
        painter.drawText(move_box.hitbox.translated(10, 0), Qt::AlignLeft | Qt::AlignVCenter,
                         QString::fromStdString(move_box.node.move.to_san()));

        font.setPointSize(7);
        font.setItalic(true);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(Qt::red);
        painter.drawText(move_box.hitbox.translated(10, 0), Qt::AlignLeft | Qt::AlignBottom,
                         QString::number(move_box.node.move_id).left(3));

        font.setBold(false);
        font.setItalic(false);
        font.setPointSize(12);
        painter.setFont(font);
        painter.setPen(QColor(77, 77, 77));
    }
}

void NotationView::resizeEvent(QResizeEvent *event) {}

void NotationView::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    switch (key) {
        case Qt::Key_Left:
            back();
            break;
        case Qt::Key_Right:
            forward();
            break;
    }
}
