#include "boardview.hxx"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <iostream>
#include <random>

#include "types.hxx"

BoardView::BoardView(QWidget *parent)
    : QWidget{parent}
    , m_square_size(100)
    , m_flipped(false)
    , m_selected_square(db::SQUARE_NONE)
    , m_arrow_drag_start(db::SQUARE_NONE)
    , m_arrow_drag_end(db::SQUARE_NONE)
    , m_pressed_piece_no_drag(false)
    , m_is_dragging(false)
    , m_left_square(false)
    , m_prefer_dragging(true)
    , m_should_deselect(false)
    , m_is_animating(false)
    , m_piece_move_animation_val(0)
    , m_piece_enlarge_val(0)
    , m_piece_dwindle_val(0)
    , m_is_promoting(false)
    , m_promotion_square(db::SQUARE_NONE)
    , m_promotion_piece_size(0.875)
    , m_promotion_chooser_cur_hovered_square(db::SQUARE_NONE)
    , m_promotion_chooser_last_hovered_square(db::SQUARE_NONE)
    , m_shape_green(QColor(21, 120, 27, 255))
    , m_shape_red(QColor(136, 32, 32, 255))
    , m_shape_blue(QColor(0, 48, 136, 255))
    , m_shape_yellow(QColor(230, 143, 0, 255))
    , m_modifiers(Qt::NoModifier)
    , m_fast_last_move(false)
{
    m_pieces_svg[db::WHITE_PAWN].load(QStringLiteral(":/images/pieces/wP.svg"));
    m_pieces_svg[db::WHITE_KNIGHT].load(QStringLiteral(":/images/pieces/wN.svg"));
    m_pieces_svg[db::WHITE_BISHOP].load(QStringLiteral(":/images/pieces/wB.svg"));
    m_pieces_svg[db::WHITE_ROOK].load(QStringLiteral(":/images/pieces/wR.svg"));
    m_pieces_svg[db::WHITE_QUEEN].load(QStringLiteral(":/images/pieces/wQ.svg"));
    m_pieces_svg[db::WHITE_KING].load(QStringLiteral(":/images/pieces/wK.svg"));
    m_pieces_svg[db::BLACK_PAWN].load(QStringLiteral(":/images/pieces/bP.svg"));
    m_pieces_svg[db::BLACK_KNIGHT].load(QStringLiteral(":/images/pieces/bN.svg"));
    m_pieces_svg[db::BLACK_BISHOP].load(QStringLiteral(":/images/pieces/bB.svg"));
    m_pieces_svg[db::BLACK_ROOK].load(QStringLiteral(":/images/pieces/bR.svg"));
    m_pieces_svg[db::BLACK_QUEEN].load(QStringLiteral(":/images/pieces/bQ.svg"));
    m_pieces_svg[db::BLACK_KING].load(QStringLiteral(":/images/pieces/bK.svg"));
    // m_board_svg.load(QString(":/images/boards/brown.svg"));
    m_board_pixmap_high_quality = QPixmap(QStringLiteral(":images/boards/pink-pyramid.png"));

    setMouseTracking(true);

    connect(this, &BoardView::move_made, this, [&](const db::Move &move) { m_last_move = move; });

    // for (db::Square square = db::A1; square <= db::H8; ++square) {
    //     m_circles[square] = QColor(random() % 255, random() % 255, random() % 255, 255);

    // }
    // for (db::Square from = db::A1; from <= db::H8; ++from) {
    //     for (db::Square to = db::A1; to <= db::H8; ++to) {
    //         if (from != to)
    //             m_arrows.push_back(Arrow{from, to, QColor(random() % 255, random() % 255, random() % 255, random() %
    //             255)});
    //     }
    // }
    setFocusPolicy(Qt::StrongFocus);
}
static std::random_device s_random_device;
static std::mt19937_64 s_engine(s_random_device());
void BoardView::paintEvent(QPaintEvent *event)
{
    if (!m_is_animating) {
        recalculate_figurines();
        process_animation_queue();
    }
    draw_board(event);
    draw_effects(event);
    draw_coordinates(event);
    draw_pieces(event);
    draw_shapes(event);
    if (m_is_promoting)
        draw_promotion(event);

    QPainter painter(this);
    if (m_is_dragging && m_board.get_piece_at(m_selected_square) != db::PIECE_NONE)
        painter.drawPixmap(m_cursor_pos, m_pieces_pixmap[m_board.get_piece_at(m_selected_square)]);
}

void BoardView::repaint_pieces()
{
    for (int i = 0; i < 12; ++i) {
        QPixmap pm(m_square_size, m_square_size);
        pm.fill(Qt::transparent);
        QPainter painter(&pm);
        m_pieces_svg[i].render(&painter, QRectF(0, 0, m_square_size, m_square_size));
        m_pieces_pixmap[i] = pm;
    }
    // QPixmap pm(m_square_size*8, m_square_size*8);
    // QPainter painter(&pm);
    // m_board_svg.render(&painter, QRectF(0,0,m_square_size * 8, m_square_size * 8));
    m_board_pixmap = m_board_pixmap_high_quality.scaled(QSize(m_square_size * 8, m_square_size * 8));
}

void BoardView::recalculate_figurines()
{
    m_figurines.clear();
    for (db::Square sq = db::A1; sq <= db::H8; ++sq) {
        db::Piece piece = m_board.get_piece_at(sq);
        if (piece != db::PIECE_NONE) {
            Figurine fg = {piece, sq, db::SQUARE_NONE, false, m_is_dragging && m_selected_square == sq, false, false};
            m_figurines.push_back(fg);
        }
    }
}
void BoardView::draw_board(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_board_pixmap);
}

void BoardView::draw_coordinates(QPaintEvent *event)
{
    std::array<char, 8> files = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    QPainter painter(this);
    for (db::Square sq = db::A1; sq <= db::H8; ++sq) {
        QPointF pos = point_at(sq);
        QFont font(QStringLiteral("Noto Sans"));
        font.setPixelSize(m_square_size / 8);
        font.setBold(true);
        painter.setFont(font);
        if (m_flipped ? rank_of(sq) == db::RANK_8 : rank_of(sq) == db::RANK_1) {
            if (m_flipped) {
                if (file_of(sq) % 2 == 1)
                    // painter.setPen(QColor(240, 217, 182, 255));
                    painter.setPen(QColor(232, 233, 183, 255));
                else
                    // painter.setPen(QColor(181, 136, 99, 255));
                    painter.setPen(QColor(237, 114, 114, 255));
            } else {
                if (file_of(sq) % 2 == 0)
                    // painter.setPen(QColor(240, 217, 182, 255));
                    painter.setPen(QColor(232, 233, 183, 255));
                else
                    // painter.setPen(QColor(181, 136, 99, 255));
                    painter.setPen(QColor(237, 114, 114, 255));
            }
            painter.drawText(pos + QPointF(4, m_square_size - 0.4 * m_square_size / 8), QString(files[file_of(sq)]));
        }
        if (m_flipped ? file_of(sq) == db::FILE_A : file_of(sq) == db::FILE_H) {
            if (m_flipped) {
                if (rank_of(sq) % 2 == 1)
                    // painter.setPen(QColor(181, 136, 99, 255));
                    painter.setPen(QColor(237, 114, 114, 255));
                else
                    // painter.setPen(QColor(240, 217, 182, 255));
                    painter.setPen(QColor(232, 233, 183, 255));
            } else {
                if (rank_of(sq) % 2 == 0)
                    // painter.setPen(QColor(181, 136, 99, 255));
                    painter.setPen(QColor(237, 114, 114, 255));
                else
                    // painter.setPen(QColor(240, 217, 182, 255));
                    painter.setPen(QColor(232, 233, 183, 255));
            }

            painter.drawText(pos + QPointF(m_square_size - 0.8 * m_square_size / 8, m_square_size / 8 + 4),
                             QString::number(rank_of(sq) + 1));
        }
    }
}

void BoardView::draw_effects(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    for (db::Square sq = db::A1; sq <= db::H8; ++sq) {
        QPointF pos = point_at(sq);
        db::Piece piece = m_board.get_piece_at(sq);
        if (m_board.is_check() && db::type_of(piece) == db::PieceType::KING &&
            db::color_of(piece) == m_board.get_stm()) {
            QRadialGradient check_indicator(QPointF(pos.x() + m_square_size / 2, pos.y() + m_square_size / 2),
                                            sqrt((m_square_size / 2) * (m_square_size / 2) * 2));
            // check_indicator.setColorAt(0,Qt::red);
            // check_indicator.setColorAt(1,Qt::transparent);
            check_indicator.setColorAt(0, QColor(255, 0, 0, 255));
            check_indicator.setColorAt(0.25, QColor(231, 0, 0, 255));
            check_indicator.setColorAt(0.89, QColor(169, 0, 0, 0));
            check_indicator.setColorAt(1, QColor(158, 0, 0, 0));
            painter.fillRect(QRectF(pos, QSizeF(m_square_size, m_square_size)), check_indicator);
        }
        if (sq == m_selected_square && !m_is_dragging) {
            painter.fillRect(QRectF(pos, QSizeF(m_square_size, m_square_size)), QBrush(QColor(20, 85, 30, 255 / 2)));
        }
        if (sq == m_selected_square && m_is_dragging && piece != db::PIECE_NONE) {
            painter.fillRect(QRectF(pos, QSizeF(m_square_size, m_square_size)), QBrush(QColor(20, 85, 30, 255 / 2)));
            painter.setOpacity(0.3);
            painter.drawPixmap(QRect(pos.toPoint(), QSize(m_square_size, m_square_size)), m_pieces_pixmap[piece]);
            painter.setOpacity(1.0);
            continue;
        }
        // if(piece != db::PIECE_NONE) {
        // painter.drawPixmap(pos, m_pieces_pixmap[piece]);
        // }
        if (!m_is_promoting) {
            if (m_board.is_piece_attack(m_selected_square, sq)) {
                if (piece == db::PIECE_NONE) {
                    QPointF circle_pos = pos;
                    circle_pos.setX(circle_pos.x() + m_square_size / 4 + m_square_size / 8);
                    circle_pos.setY(circle_pos.y() + m_square_size / 4 + m_square_size / 8);
                    painter.setPen(Qt::transparent);
                    painter.setBrush(QColor(20, 85, 30, 128));
                    painter.drawEllipse(QRectF(circle_pos, QSizeF(m_square_size / 4, m_square_size / 4)));
                } else {
                    QPointF circle_pos = pos;
                    circle_pos.setX(circle_pos.x() - m_square_size / 16);
                    circle_pos.setY(circle_pos.y() - m_square_size / 16);
                    QPainterPath path;
                    QPainterPath bounds;
                    path.addRect(QRectF(pos, QSizeF(m_square_size, m_square_size)));
                    path.addEllipse(QRectF(
                        circle_pos, QSizeF(m_square_size + m_square_size / 8, m_square_size + m_square_size / 8)));
                    bounds.addRect(QRectF(pos, QSizeF(m_square_size, m_square_size)));
                    // bounds.addRect(QRectF(pos, QSizeF(m_square_size/4, m_square_size/4)));
                    // bounds.addRect(QRectF(QPointF(pos.x() + m_square_size - m_square_size/4, pos.y()),
                    // QSizeF(m_square_size/4, m_square_size/4))); bounds.addRect(QRectF(QPointF(pos.x(), pos.y() +
                    // m_square_size - m_square_size/4), QSizeF(m_square_size/4, m_square_size/4)));
                    // bounds.addRect(QRectF(QPointF(pos.x() + m_square_size - m_square_size/4, pos.y() + m_square_size
                    // - m_square_size/4), QSizeF(m_square_size/4, m_square_size/4)));
                    painter.setPen(Qt::transparent);
                    painter.setBrush(QColor(20, 85, 30, 128));
                    // painter.drawPath(path);
                    painter.drawPath(path.intersected(bounds));
                    // painter.fillPath(bounds, QColor(Qt::red));
                }

                if (m_board.is_piece_attack(m_selected_square,
                                            square_at(QPointF(m_cursor_pos.x() + m_square_size / 2,
                                                              m_cursor_pos.y() + m_square_size / 2))) &&
                    sq == square_at(
                              QPointF(m_cursor_pos.x() + m_square_size / 2, m_cursor_pos.y() + m_square_size / 2))) {
                    painter.fillRect(QRectF(pos, QSizeF(m_square_size, m_square_size)), QBrush(QColor(20, 85, 30, 77)));
                }
            }
        }
        if (sq == m_last_move.from || sq == m_last_move.to)
            painter.fillRect(QRectF(pos, QSizeF(m_square_size, m_square_size)),
                             QBrush(QColor(155, 199, 0, 0.41 * 255)));
    }
}

void BoardView::draw_pieces(QPaintEvent *event)
{
    QPainter painter(this);
    for (auto f : m_figurines) {
        QPointF pos = point_at(f.pos);
        if (!f.dragged && !f.animating && !f.fade && !f.ghost)
            painter.drawPixmap(pos, m_pieces_pixmap[f.piece]);
        else if (f.ghost) {
            painter.setOpacity(0.3);
            painter.drawPixmap(pos, m_pieces_pixmap[f.piece]);
            painter.setOpacity(1.0);
        }
    }
    for (auto f : m_figurines) {
        QPointF pos = point_at(f.pos);
        QPointF target = point_at(f.target);
        if (f.animating)
            painter.drawPixmap(pos + (target - pos) * m_piece_move_animation_val, m_pieces_pixmap[f.piece]);
        else if (f.fade) {
            painter.setOpacity(1 - m_piece_move_animation_val);
            painter.drawPixmap(pos, m_pieces_pixmap[f.piece]);
            painter.setOpacity(1.0);
        }
    }
    if (m_piece_move_animation_val == 1.0) {
        m_is_animating = false;
        m_piece_move_animation_val = 0;
        update();
    }
}

void BoardView::draw_shapes(QPaintEvent *event)
{
    QColor color;
    if (m_modifiers == Qt::NoModifier) {
        color = m_shape_green;
    } else if (m_modifiers == Qt::ShiftModifier || m_modifiers == Qt::ControlModifier) {
        color = m_shape_red;
    } else if (m_modifiers == Qt::AltModifier) {
        color = m_shape_blue;
    } else if (m_modifiers == (Qt::AltModifier | Qt::ShiftModifier) ||
               m_modifiers == (Qt::AltModifier | Qt::ControlModifier)) {
        color = m_shape_yellow;
    }

    QPixmap pixmap(m_square_size * 8, m_square_size * 8);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    for (auto &m_circle : m_circles) {
        QPointF pos = point_at(m_circle.first);
        painter.setPen(Qt::transparent);
        painter.setBrush(m_circle.second);
        QPainterPath circle;
        circle.addEllipse(QRectF(pos, QSizeF(m_square_size, m_square_size)));
        circle.addEllipse(QRectF(pos + QPointF(m_square_size / 20, m_square_size / 20),
                                 QSize(m_square_size - m_square_size / 10, m_square_size - m_square_size / 10)));
        painter.drawPath(circle);
    }
    std::vector<db::Square> seen;
    seen.reserve(m_arrows.size());
    std::vector<db::Square> duplicates;
    for (auto &m_arrow : m_arrows) {
        if (std::find(seen.begin(), seen.end(), m_arrow.to) != seen.end() &&
            std::find(duplicates.begin(), duplicates.end(), m_arrow.to) == duplicates.end()) {
            duplicates.push_back(m_arrow.to);
        }
        seen.push_back(m_arrow.to);
    }
    if (m_arrow_drag_start != m_arrow_drag_end && m_arrow_drag_start != db::SQUARE_NONE &&
        m_arrow_drag_end != db::SQUARE_NONE) {
        if (std::find(seen.begin(), seen.end(), m_arrow_drag_end) != seen.end() &&
            std::find(duplicates.begin(), duplicates.end(), m_arrow_drag_end) == duplicates.end()) {
            duplicates.push_back(m_arrow_drag_end);
        }
    }

    for (auto &m_arrow : m_arrows) {
        QPointF p1 = point_at(m_arrow.from);
        QPointF p2 = point_at(m_arrow.to);

        if (std::find(duplicates.begin(), duplicates.end(), m_arrow.to) != duplicates.end()) {
            float dy = p2.y() - p1.y();
            float dx = p2.x() - p1.x();
            float length = sqrtf(dx * dx + dy * dy);

            draw_arrow(&painter, p1 + QPointF(m_square_size / 2, m_square_size / 2),
                       p2 + QPointF(m_square_size / 2, m_square_size / 2) -
                           m_square_size / 8 * QPointF(dx / length, dy / length),
                       m_square_size / 8, m_square_size / 3.5f, m_square_size / 2.2f, m_arrow.color);
        } else {
            draw_arrow(&painter, p1 + QPointF(m_square_size / 2, m_square_size / 2),
                       p2 + QPointF(m_square_size / 2, m_square_size / 2), m_square_size / 8, m_square_size / 3.5f,
                       m_square_size / 2.2f, m_arrow.color);
        }
    }
    if (m_arrow_drag_start != m_arrow_drag_end && m_arrow_drag_start != db::SQUARE_NONE &&
        m_arrow_drag_end != db::SQUARE_NONE) {
        QPointF p1 = point_at(m_arrow_drag_start);
        QPointF p2 = point_at(m_arrow_drag_end);
        if (std::find(duplicates.begin(), duplicates.end(), m_arrow_drag_end) != duplicates.end()) {
            float dy = p2.y() - p1.y();
            float dx = p2.x() - p1.x();
            float length = sqrtf(dx * dx + dy * dy);
            draw_arrow(&painter, p1 + QPointF(m_square_size / 2, m_square_size / 2),
                       p2 + QPointF(m_square_size / 2, m_square_size / 2) -
                           m_square_size / 8 * QPointF(dx / length, dy / length),
                       m_square_size / 10, m_square_size / 4, m_square_size / 2.5f, color);
        } else {
            draw_arrow(&painter, p1 + QPointF(m_square_size / 2, m_square_size / 2),
                       p2 + QPointF(m_square_size / 2, m_square_size / 2), m_square_size / 10, m_square_size / 4,
                       m_square_size / 2.5f, color);
        }

    } else if (m_arrow_drag_start != db::SQUARE_NONE) {
        QPointF pos = point_at(m_arrow_drag_start);
        painter.setPen(Qt::transparent);
        painter.setBrush(color);
        QPainterPath circle;
        circle.addEllipse(QRectF(pos, QSizeF(m_square_size, m_square_size)));
        circle.addEllipse(QRectF(pos + QPointF(m_square_size / 25, m_square_size / 25),
                                 QSize(m_square_size - m_square_size / 12.5f, m_square_size - m_square_size / 12.5f)));
        painter.drawPath(circle);
    }

    // std::vector<db::Move> moves = m_board.generate_moves(m_board.get_position());

    // for (auto it = moves.begin(); it != moves.end(); ++it) {
    //     QPointF p1 = point_at(it->from);
    //     QPointF p2 = point_at(it->to);
    //     draw_arrow(&painter, p1 + QPointF(m_square_size/2, m_square_size/2),
    //     p2 + QPointF(m_square_size/2, m_square_size/2), m_square_size/10, m_square_size/4, m_square_size/2.5,
    //     m_board.get_stm() == db::BLACK ? m_shape_red : m_shape_blue);
    // }

    QPainter p(this);
    p.setOpacity(0.6);
    p.drawPixmap(QPointF(0, 0), pixmap);
    p.setOpacity(1);
}

void BoardView::draw_promotion(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    std::array<db::Piece, 4> w_promotion_option = {db::WHITE_QUEEN, db::WHITE_KNIGHT, db::WHITE_ROOK, db::WHITE_BISHOP};
    std::array<db::Piece, 4> b_promotion_option = {db::BLACK_QUEEN, db::BLACK_KNIGHT, db::BLACK_ROOK, db::BLACK_BISHOP};

    painter.fillRect(0, 0, m_square_size * 8, m_square_size * 8, QColor(255, 255, 255, 255 / 2));
    if (rank_of(m_promotion_square) == db::RANK_1) {
        for (int i = 0; i < 4; ++i) {
            db::Square sq = db::make_square(file_of(m_promotion_square), db::Rank(rank_of(m_promotion_square) + i));
            float promotion_piece_size = 0.875;
            float radius = m_square_size / 2;
            if (sq == m_promotion_chooser_cur_hovered_square) {
                promotion_piece_size = 0.875 + 0.125 * m_piece_enlarge_val;
                radius = (1 - m_piece_enlarge_val) * m_square_size;
            } else if (sq == m_promotion_chooser_last_hovered_square) {
                promotion_piece_size = 0.875 + 0.125 * m_piece_dwindle_val;
                radius = (1 - m_piece_dwindle_val) * m_square_size;
            }
            QPointF pos = point_at(sq);
            painter.setPen(Qt::transparent);
            QRadialGradient gradient(QPointF(pos.x() + m_square_size / 2, pos.y() + m_square_size / 2),
                                     sqrt((m_square_size / 2) * (m_square_size / 2) * 2));
            if (sq == m_promotion_chooser_cur_hovered_square) {
                gradient.setColorAt(0, QColor(196, 196, 196, 255));
                gradient.setColorAt(0.7, QColor(200, 100, 50, 255));
                gradient.setColorAt(1, QColor(205, 70, 37, 255));
            } else {
                gradient.setColorAt(0, QColor(196, 196, 196, 255));
                gradient.setColorAt(0.7, QColor(128, 128, 128, 255));
            }
            painter.setBrush(gradient);
            painter.drawRoundedRect(QRectF(pos, QSizeF(m_square_size, m_square_size)), radius, radius);
            // painter.drawPixmap(pos, m_pieces_pixmap[b_promotion_option[i]]);
            m_pieces_svg[b_promotion_option[i]].render(
                &painter, QRectF(pos + QPointF(m_square_size * (1 - promotion_piece_size) / 2,
                                               m_square_size * (1 - promotion_piece_size) / 2),
                                 QSizeF(m_square_size * promotion_piece_size, m_square_size * promotion_piece_size)));
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            db::Square sq = db::make_square(file_of(m_promotion_square), db::Rank(rank_of(m_promotion_square) - i));
            float promotion_piece_size = 0.875;
            float radius = m_square_size / 2;
            if (sq == m_promotion_chooser_cur_hovered_square) {
                promotion_piece_size = 0.875 + 0.125 * m_piece_enlarge_val;
                radius = (1 - m_piece_enlarge_val) * m_square_size;
            } else if (sq == m_promotion_chooser_last_hovered_square) {
                promotion_piece_size = 0.875 + 0.125 * m_piece_dwindle_val;
                radius = (1 - m_piece_dwindle_val) * m_square_size;
            }
            QPointF pos = point_at(sq);
            painter.setPen(Qt::transparent);
            QRadialGradient gradient(QPointF(pos.x() + m_square_size / 2, pos.y() + m_square_size / 2),
                                     sqrtf((m_square_size / 2) * (m_square_size / 2) * 2));
            if (sq == m_promotion_chooser_cur_hovered_square) {
                gradient.setColorAt(0, QColor(196, 196, 196, 255));
                gradient.setColorAt(0.7, QColor(200, 100, 50, 255));
                gradient.setColorAt(1, QColor(205, 70, 37, 255));
            } else {
                gradient.setColorAt(0, QColor(196, 196, 196, 255));
                gradient.setColorAt(0.7, QColor(128, 128, 128, 255));
            }
            painter.setBrush(gradient);
            painter.drawRoundedRect(QRectF(pos, QSizeF(m_square_size, m_square_size)), radius, radius);
            // painter.drawPixmap(pos, m_pieces_pixmap[b_promotion_option[i]]);
            m_pieces_svg[w_promotion_option[i]].render(
                &painter, QRectF(pos + QPointF(m_square_size * (1 - promotion_piece_size) / 2,
                                               m_square_size * (1 - promotion_piece_size) / 2),
                                 QSizeF(m_square_size * promotion_piece_size, m_square_size * promotion_piece_size)));
        }
    }
}

void BoardView::draw_arrow(QPainter *painter, QPointF start, QPointF end, float line_width, float arrow_width,
                           float arrow_height, QColor color)
{
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setBrush(color);
    painter->setPen(Qt::transparent);

    float dx = end.x() - start.x();
    float dy = end.y() - start.y();
    float length = sqrtf(dx * dx + dy * dy);

    float normal_x = dx / length;
    float normal_y = dy / length;

    QPointF normal_p = QPointF(normal_x, normal_y);

    float perpendicular_x = -normal_y;
    float perpendicular_y = normal_x;

    QPointF perpendicular_p = QPointF(perpendicular_x, perpendicular_y);

    QPointF p2 = end - normal_p * arrow_height + perpendicular_p * arrow_width;
    QPointF p3 = end - normal_p * arrow_height - perpendicular_p * arrow_width;
    QPointF p4 = start + perpendicular_p * line_width / 2;
    QPointF p5 = start + perpendicular_p * line_width / 2 + normal_p * (length - arrow_height);
    QPointF p6 = start - perpendicular_p * line_width / 2 + normal_p * (length - arrow_height);
    QPointF p7 = start - perpendicular_p * line_width / 2;

    QPolygonF arrow = {p4, p5, p2, end, p3, p6, p7};
    painter->drawPolygon(arrow);
    painter->drawEllipse(QRectF(start - QPointF(line_width / 2, line_width / 2), QSizeF(line_width, line_width)));
}

void BoardView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!m_is_promoting) {
            db::Square square = square_at(event->position());
            db::Piece piece = m_board.get_piece_at(square);
            m_press_pos = event->position();
            m_pressed_piece_no_drag = false;
            if (piece != db::PIECE_NONE && db::color_of(piece) == m_board.get_stm()) {
                if (m_selected_square == square) {
                    m_should_deselect = true;
                } else {
                    m_selected_square = square;
                    m_should_deselect = false;
                }
                if (m_prefer_dragging) {
                    m_cursor_pos =
                        QPointF(event->position().x() - m_square_size / 2, event->position().y() - m_square_size / 2);
                    m_is_dragging = true;
                    m_left_square = false;
                } else {
                    m_pressed_piece_no_drag = true;
                }
            } else if (m_board.is_piece_attack(m_selected_square, square)) {
                db::Piece promoted = db::PIECE_NONE;
                if (db::type_of(m_board.get_piece_at(m_selected_square)) == db::PAWN &&
                    (db::rank_of(square) == db::RANK_8 || db::rank_of(square) == db::RANK_1)) {
                    m_is_promoting = true;
                    m_promotion_square = square;
                    m_promotion_chooser_cur_hovered_square = square_at(event->pos());
                    m_piece_enlarge_animation = new QPropertyAnimation(this, "piece_enlarge_animation", this);
                    m_piece_enlarge_animation->setStartValue(0);
                    m_piece_enlarge_animation->setEndValue(1);
                    m_piece_enlarge_animation->setDuration(200);
                    m_piece_enlarge_animation->start();
                    return;
                }

                db::Move move = make_move(m_selected_square, square, db::PIECE_NONE);
                db::Position from = m_board.get_position();
                move = m_board.prepare_move(move);
                if (move.is_legal) {
                    m_prefer_dragging = false;
                    window()->setWindowTitle(QString::fromStdString(db::color_str.at(m_board.get_stm())) +
                                             QStringLiteral(" to move"));
                    m_board.prepare_for_print(move);
                    emit move_made(move);
                    emit fen_changed(m_board.get_fen());
                    // if(move.is_castling)
                    //     start_animation(from, m_board.get_position());
                    // else
                    // queue_move_animation(0, move);
                }
                m_selected_square = db::SQUARE_NONE;
            }

            else {
                m_selected_square = db::SQUARE_NONE;
            }
        } else {
            db::Piece promotion = db::PIECE_NONE;
            if (rank_of(m_promotion_square) == db::RANK_1) {
                db::Square pressed_square = square_at(event->position());
                db::File promotion_file = db::file_of(m_promotion_square);
                if (file_of(pressed_square) == promotion_file) {
                    switch (rank_of(pressed_square)) {
                        case db::RANK_1:
                            promotion = db::BLACK_QUEEN;
                            break;
                        case db::RANK_2:
                            promotion = db::BLACK_KNIGHT;
                            break;
                        case db::RANK_3:
                            promotion = db::BLACK_ROOK;
                            break;
                        case db::RANK_4:
                            promotion = db::BLACK_BISHOP;
                            break;
                        default:
                            m_is_promoting = false;
                            m_promotion_square = db::SQUARE_NONE;
                            m_selected_square = db::SQUARE_NONE;
                            return; // canceled
                    }

                } else {
                    m_is_promoting = false;
                    m_promotion_square = db::SQUARE_NONE;
                    m_selected_square = db::SQUARE_NONE;
                    return; // canceled
                }
            } else {
                db::Square pressed_square = square_at(event->position());
                db::File promotion_file = db::file_of(m_promotion_square);
                if (file_of(pressed_square) == promotion_file) {
                    switch (rank_of(pressed_square)) {
                        case db::RANK_8:
                            promotion = db::WHITE_QUEEN;
                            break;
                        case db::RANK_7:
                            promotion = db::WHITE_KNIGHT;
                            break;
                        case db::RANK_6:
                            promotion = db::WHITE_ROOK;
                            break;
                        case db::RANK_5:
                            promotion = db::WHITE_BISHOP;
                            break;
                        default:
                            m_is_promoting = false;
                            m_promotion_square = db::SQUARE_NONE;
                            m_selected_square = db::SQUARE_NONE;
                            return; // canceled
                    }
                } else {
                    m_is_promoting = false;
                    m_promotion_square = db::SQUARE_NONE;
                    m_selected_square = db::SQUARE_NONE;
                    return; // canceled
                }
            }
            db::Move move = make_move(m_selected_square, m_promotion_square, promotion);
            db::Position from = m_board.get_position();
            move = m_board.prepare_move(move);
            // bool moved = m_board.do_move(move);
            if (move.is_legal) {
                m_prefer_dragging = false;
                window()->setWindowTitle(QString::fromStdString(db::color_str.at(m_board.get_stm())) +
                                         QStringLiteral(" to move"));
                m_board.prepare_for_print(move);
                emit move_made(move);
                emit fen_changed(m_board.get_fen());
                // queue_move_animation(0,move);
                // start_animation(from, m_board.get_position());
            }
            m_selected_square = db::SQUARE_NONE;
            m_is_promoting = false;
            m_promotion_square = db::SQUARE_NONE;
        }
    } else if (event->button() == Qt::RightButton) {
        m_modifiers = event->modifiers();
        m_arrow_drag_start = square_at(event->position());
    }
    update();
}

void BoardView::mouseMoveEvent(QMouseEvent *event)
{
    m_cursor_pos = QPointF(event->position().x() - m_square_size / 2, event->position().y() - m_square_size / 2);
    if (!m_is_promoting) {
        if (m_pressed_piece_no_drag &&
            sqrt((event->position() - m_press_pos).x() * (event->position() - m_press_pos).x() +
                 (event->position() - m_press_pos).y() * (event->position() - m_press_pos).y()) >= 3 &&
            !m_is_dragging) {
            m_is_dragging = true;
        }
        if (m_is_dragging && square_at(event->position()) != m_selected_square) {
            m_left_square = true;
        }
    } else {
        if (square_at(event->position()) != m_promotion_chooser_cur_hovered_square) {
            m_promotion_chooser_last_hovered_square = m_promotion_chooser_cur_hovered_square;
            m_promotion_chooser_cur_hovered_square = square_at(event->position());
            m_piece_enlarge_animation = new QPropertyAnimation(this, "piece_enlarge_animation", this);
            m_piece_enlarge_animation->setStartValue(0);
            m_piece_enlarge_animation->setEndValue(1);
            m_piece_enlarge_animation->setDuration(200);
            m_piece_enlarge_animation->start();
            if (m_promotion_chooser_cur_hovered_square != m_promotion_chooser_last_hovered_square &&
                m_promotion_chooser_last_hovered_square != db::SQUARE_NONE) {
                m_piece_dwindle_animation = new QPropertyAnimation(this, "piece_dwindle_animation", this);
                m_piece_dwindle_animation->setStartValue(1);
                m_piece_dwindle_animation->setEndValue(0);
                m_piece_dwindle_animation->setDuration(200);
                m_piece_dwindle_animation->start();
            }
        }
    }
    if (event->buttons() == Qt::RightButton) {
        m_arrow_drag_end = square_at(event->position());
    }
    update();
}

void BoardView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed_piece_no_drag = false;
        db::Square square = square_at(event->position());
        if (m_is_dragging && m_left_square && m_board.is_piece_attack(m_selected_square, square)) {
            if (db::type_of(m_board.get_piece_at(m_selected_square)) == db::PAWN &&
                (db::rank_of(square) == db::RANK_8 || db::rank_of(square) == db::RANK_1)) {
                m_is_promoting = true;
                m_promotion_square = square;
                m_is_dragging = false;
                m_promotion_chooser_cur_hovered_square = square_at(event->pos());
                m_piece_enlarge_animation = new QPropertyAnimation(this, "piece_enlarge_animation", this);
                m_piece_enlarge_animation->setStartValue(0);
                m_piece_enlarge_animation->setEndValue(1);
                m_piece_enlarge_animation->setDuration(200);
                m_piece_enlarge_animation->start();
                update();
                return;
            }

            db::Move move = make_move(m_selected_square, square, db::PIECE_NONE);
            move = m_board.prepare_move(move);
            if (move.is_legal) {
                m_prefer_dragging = true;
                window()->setWindowTitle(QString::fromStdString(db::color_str.at(m_board.get_stm())) +
                                         QStringLiteral(" to move"));
                m_board.prepare_for_print(move);
                emit move_made(move, false);
                emit fen_changed(m_board.get_fen());
            }
            m_selected_square = db::SQUARE_NONE;

        } else if (m_is_dragging && m_left_square) {
            m_selected_square = db::SQUARE_NONE;
        }
        m_is_dragging = false;
        if (m_should_deselect && m_selected_square != db::SQUARE_NONE && m_selected_square == square) {
            m_selected_square = db::SQUARE_NONE;
        }
    } else if (event->button() == Qt::RightButton) {
        QColor color;
        if (m_modifiers == Qt::NoModifier) {
            color = m_shape_green;
        } else if (m_modifiers == Qt::ShiftModifier || m_modifiers == Qt::ControlModifier) {
            color = m_shape_red;
        } else if (m_modifiers == Qt::AltModifier) {
            color = m_shape_blue;
        } else if (m_modifiers == (Qt::AltModifier | Qt::ShiftModifier) ||
                   m_modifiers == (Qt::AltModifier | Qt::ControlModifier)) {
            color = m_shape_yellow;
        }

        QColor erased_color;

        if (m_arrow_drag_start != m_arrow_drag_end && m_arrow_drag_start != db::SQUARE_NONE &&
            m_arrow_drag_end != db::SQUARE_NONE) {
            bool erased = false;
            for (auto it = m_arrows.begin(); it != m_arrows.end(); ++it) {
                if (it->from == m_arrow_drag_start && it->to == m_arrow_drag_end) {
                    erased_color = it->color;
                    m_arrows.erase(it);
                    erased = true;
                    break;
                }
            }
            if (!erased || erased_color != color)
                m_arrows.push_back(Arrow{m_arrow_drag_start, m_arrow_drag_end, color});
        } else if (m_arrow_drag_start != db::SQUARE_NONE) {
            if (m_circles.find(m_arrow_drag_start) == m_circles.end())
                m_circles[m_arrow_drag_start] = color;
            else
                m_circles.erase(m_arrow_drag_start);
        }
        m_arrow_drag_start = db::SQUARE_NONE;
        m_arrow_drag_end = db::SQUARE_NONE;
    }
    m_modifiers = Qt::NoModifier;
    update();
}

void BoardView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F) {
        m_flipped = m_flipped ? false : true;
    } else if (event->key() == Qt::Key_Left) {
        emit back();
    } else if (event->key() == Qt::Key_Right) {
        emit forward();
    } else if (event->key() == Qt::Key_Space) {
        db::Square from = db::SQUARE_NONE;
        db::Square to = db::SQUARE_NONE;
        bool legal = false;
        std::vector<db::Move> moves = m_board.generate_moves(m_board.get_position());
        db::Move move;
        if (moves.size() != 0) {
            std::uniform_int_distribution<uint> dist(0, moves.size() - 1);
            move = moves.at(dist(s_engine));
        } else
            return;
        move = m_board.prepare_move(move);
        m_board.prepare_for_print(move);
        emit move_made(move, false);
        emit fen_changed(m_board.get_fen());
    }
    update();
}

void BoardView::resizeEvent(QResizeEvent *event)
{
    m_square_size = (width() > height() ? height() : width()) / 8;
    repaint_pieces();
}

db::Square BoardView::square_at(const QPointF point)
{
    int x = point.x();
    int y = point.y();
    if (x <= 0 || y <= 0 || x >= m_square_size * 8 || y >= m_square_size * 8) {
        return db::SQUARE_NONE;
    }
    x /= m_square_size;
    y /= m_square_size;
    return db::make_square(db::File(m_flipped ? 7 - x : x), db::Rank(m_flipped ? y : 7 - y));
}

QPointF BoardView::point_at(const db::Square square)
{
    int x = m_flipped ? 7 - square % 8 : square % 8;
    int y = m_flipped ? square / 8 : 7 - square / 8;
    return {x * m_square_size, y * m_square_size};
}

void BoardView::forward_move(const db::MoveId id, const db::Move &move)
{
    queue_move_animation(id, move);
}

void BoardView::back_move(const db::MoveId id, const db::Move &move)
{
    queue_move_animation(id, move, true);
}

db::Move BoardView::make_move(db::Square from, db::Square to, db::Piece promotion)
{
    db::Move move;
    move.from = from;
    move.to = to;
    move.piece_moved = m_board.get_piece_at(from);
    move.captured = m_board.get_piece_at(to);
    move.color = db::color_of(m_board.get_piece_at(from));
    move.promoted = promotion;
    move.is_enpassant = false;
    move.is_castling = false;
    move.is_legal = false;
    move.prev_ep = m_board.get_ep_square();
    move.castling_rights = m_board.get_castling_rights();

    if (move.piece_moved == db::WHITE_KING && move.from == db::E1 &&
            (move.to == db::A1 || move.to == db::C1 || move.to == db::H1 || move.to == db::G1) ||
        move.piece_moved == db::BLACK_KING && move.from == db::E8 &&
            (move.to == db::A8 || move.to == db::C8 || move.to == db::H8 || move.to == db::G8))
        move.is_castling = true;
    return move;
}

void BoardView::start_animation(const db::Position &start, const db::Position &target)
{
    if (m_is_animating)
        return;
    std::vector<Figurine> target_figurines;
    target_figurines.reserve(12);
    for (db::Square sq = db::A1; sq <= db::H8; ++sq) {
        db::Piece piece = m_board.get_piece_at(sq, target);
        if (piece != db::PIECE_NONE) {
            Figurine fg = {piece, sq, db::SQUARE_NONE, false, m_is_dragging && m_selected_square == sq, false, false};
            target_figurines.push_back(fg);
        }
    }
    for (auto &m_figurine : m_figurines) {
        for (auto i_targets = target_figurines.begin(); i_targets != target_figurines.end();) {
            db::Square from = m_figurine.pos;
            db::Square to = i_targets->pos;

            if (m_figurine.piece == i_targets->piece &&
                m_board.get_piece_at(from, start) != m_board.get_piece_at(to, start) &&
                m_board.get_piece_at(from, start) != m_board.get_piece_at(from, target)) {
                m_figurine.target = i_targets->pos;
                m_figurine.animating = true;
                i_targets = target_figurines.erase(i_targets);
                break;
            } else {
                ++i_targets;
            }
        }
    }
    for (auto &m_figurine : m_figurines) {
        db::Square sq = m_figurine.pos;
        if (!m_figurine.animating && m_board.get_piece_at(sq, start) != m_board.get_piece_at(sq, target)) {
            m_figurine.fade = true;
        }
    }
    for (auto &target_figurine : target_figurines) {
        m_figurines.push_back(target_figurine);
    }

    m_is_animating = true;
    m_piece_move_animation = new QPropertyAnimation(this, "piece_move_animation", this);
    m_piece_move_animation->setStartValue(0.0);
    m_piece_move_animation->setEndValue(1.0);
    m_piece_move_animation->setDuration(600.0);
    m_piece_move_animation->setEasingCurve(QEasingCurve::InOutCubic);
    m_piece_move_animation->start();
}

void BoardView::start_move_animation(const db::Move &move)
{
    for (auto i_figurines = m_figurines.begin(); i_figurines != m_figurines.end();) {
        db::Square sq = i_figurines->pos;
        if (sq == move.from) {
            i_figurines->target = move.to;
            i_figurines->animating = true;
        }
        if (sq == move.to && move.captured != db::PIECE_NONE) {
            i_figurines->ghost = true;
        }
        if (move.is_enpassant && sq == (move.color == db::BLACK ? db::Square(move.to + 8) : db::Square(move.to - 8))) {
            i_figurines = m_figurines.erase(i_figurines);
        } else {
            ++i_figurines;
        }
    }
    m_is_animating = true;
    m_piece_move_animation = new QPropertyAnimation(this, "piece_move_animation", this);
    m_piece_move_animation->setStartValue(0.0);
    m_piece_move_animation->setEndValue(1.0);
    if (m_move_animation_queue.size() > 1 || m_fast_last_move) {
        m_piece_move_animation->setDuration(100.0 / m_move_animation_queue.size());
        if (!(m_move_animation_queue.size() > 1) && m_fast_last_move)
            m_fast_last_move = false;
    } else
        m_piece_move_animation->setDuration(200.0);
    m_piece_move_animation->setEasingCurve(QEasingCurve::InOutCubic);
    m_piece_move_animation->start();
}

void BoardView::start_move_undo_animation(const db::Move &move)
{
    for (auto &m_figurine : m_figurines) {
        db::Square sq = m_figurine.pos;
        if (sq == move.to) {
            if (move.promoted != db::PIECE_NONE) {
                m_figurine.piece = move.color == db::BLACK ? db::BLACK_PAWN : db::WHITE_PAWN;
            }
            m_figurine.target = move.from;
            m_figurine.animating = true;
        }
    }
    if (!move.is_enpassant && move.captured != db::PIECE_NONE)
        m_figurines.push_back(Figurine(move.captured, move.to, db::SQUARE_NONE, false, false, false, false));
    else if (move.is_enpassant)
        m_figurines.push_back(
            Figurine(move.captured, move.color == db::BLACK ? db::Square(move.to + 8) : db::Square(move.to - 8)));
    m_is_animating = true;
    m_piece_move_animation = new QPropertyAnimation(this, "piece_move_animation", this);
    m_piece_move_animation->setStartValue(0.0);
    m_piece_move_animation->setEndValue(1.0);
    if (m_move_animation_queue.size() > 1 || m_fast_last_move) {
        m_piece_move_animation->setDuration(100.0 / m_move_animation_queue.size());
        if (!(m_move_animation_queue.size() > 1) && m_fast_last_move)
            m_fast_last_move = false;
    } else
        m_piece_move_animation->setDuration(200.0);
    m_piece_move_animation->setEasingCurve(QEasingCurve::InOutCubic);
    m_piece_move_animation->start();
}

void BoardView::queue_move_animation(const db::MoveId id, const db::Move &move, bool undo)
{
    if (move == db::Move())
        return;

    m_move_animation_queue.emplace_back(id, move, undo);
    if (!m_is_animating && !std::get<bool>(m_move_animation_queue[0])) {
        m_board.do_move(std::get<db::Move>(m_move_animation_queue[0]));
        m_last_move = std::get<db::Move>(m_move_animation_queue[0]);
        start_move_animation(std::get<db::Move>(m_move_animation_queue[0]));
        emit current_move(std::get<db::MoveId>(m_move_animation_queue[0]));
        m_move_animation_queue.erase(m_move_animation_queue.begin());
    } else if (!m_is_animating) {
        m_board.undo_move(std::get<db::Move>(m_move_animation_queue[0]));
        emit get_prev_move();
        start_move_undo_animation(std::get<db::Move>(m_move_animation_queue[0]));
        emit current_move(std::get<db::MoveId>(m_move_animation_queue[0]));
        m_move_animation_queue.erase(m_move_animation_queue.begin());
    }
}

void BoardView::process_animation_queue()
{
    if (m_move_animation_queue.size() == 0)
        return;

    if (m_move_animation_queue.size() > 1) {
        m_fast_last_move = true;
    }
    if (!std::get<bool>(m_move_animation_queue[0])) {
        m_board.do_move(std::get<db::Move>(m_move_animation_queue[0]));
        m_last_move = std::get<db::Move>(m_move_animation_queue[0]);
        start_move_animation(std::get<db::Move>(m_move_animation_queue[0]));
    } else {
        m_board.undo_move(std::get<db::Move>(m_move_animation_queue[0]));
        emit get_prev_move();
        start_move_undo_animation(std::get<db::Move>(m_move_animation_queue[0]));
    }

    emit current_move(std::get<db::MoveId>(m_move_animation_queue[0]));
    m_move_animation_queue.erase(m_move_animation_queue.begin());
}

void BoardView::move_added(const db::MoveId id, const db::Move &move, bool animated)
{
    if (animated)
        queue_move_animation(id, move);
    else {
        m_board.do_move(move);
        emit current_move(id);
    }
}
