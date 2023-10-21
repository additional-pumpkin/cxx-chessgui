#pragma once
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QSvgRenderer>
#include <QWidget>
#include <vector>

#include "bitboard.hxx"
#include "movenode.hxx"
struct Figurine
{
    db::Piece piece;
    db::Square pos;
    db::Square target;
    bool fade; // for fading animation
    bool dragged;
    bool animating;
    bool ghost;
};

class BoardView : public QWidget
{
    Q_OBJECT
public:
    explicit BoardView(QWidget *parent = nullptr);

    void set_fen(const QString &fen)
    {
        if (m_board.get_fen() == fen.toStdString())
            return;
        db::Position from = m_board.get_position();
        m_board.set_fen(fen.toStdString());
        start_animation(from, m_board.get_position());
        emit fen_changed(fen.toStdString());
        update();
    }
    QString get_fen() { return QString::fromStdString(m_board.get_fen()); }

    void forward_move(const db::MoveId id, const db::Move &move);
    void back_move(const db::MoveId id, const db::Move &move);
    void move_added(const db::MoveId id, const db::Move &move, bool animated = true);
    void set_prev_move(const db::Move &move) { m_last_move = move; }

private:
    float m_square_size;
    std::array<QPixmap, 12> m_pieces_pixmap;
    std::array<QSvgRenderer, 12> m_pieces_svg;
    QPixmap m_board_pixmap;
    QPixmap m_board_pixmap_high_quality;
    QSvgRenderer m_board_svg;

    bool m_flipped;

    db::Square m_selected_square;

    db::Square m_arrow_drag_start;
    db::Square m_arrow_drag_end;

    QPointF m_cursor_pos;
    QPointF m_press_pos;
    bool m_pressed_piece_no_drag;
    bool m_is_dragging;
    bool m_left_square;
    bool m_prefer_dragging; // when it is true pieces snap to the mouse location immediately on click
    bool m_should_deselect;

    db::Move m_last_move;

    std::vector<Figurine> m_figurines;
    bool m_is_animating;
    float m_piece_move_animation_val; // from 0 to 1 - pieces moving animation
    float m_piece_enlarge_val;        // from 0 to 1 - promotion chooser piece scale up animation
    float m_piece_dwindle_val;        // from 1 to 0 - promotion chooser piece scale down animation
    QPropertyAnimation *m_piece_move_animation;
    QPropertyAnimation *m_piece_enlarge_animation;
    QPropertyAnimation *m_piece_dwindle_animation;

    Q_PROPERTY(float piece_move_animation MEMBER m_piece_move_animation_val NOTIFY update());
    Q_PROPERTY(float piece_enlarge_animation MEMBER m_piece_enlarge_val NOTIFY update());
    Q_PROPERTY(float piece_dwindle_animation MEMBER m_piece_dwindle_val NOTIFY update());

    bool m_is_promoting;
    db::Square m_promotion_square;
    float m_promotion_piece_size;
    db::Square m_promotion_chooser_cur_hovered_square;
    db::Square m_promotion_chooser_last_hovered_square;

    db::Square square_at(const QPointF point);
    QPointF point_at(const db::Square square);

    void paintEvent(QPaintEvent *event) override;
    void draw_board(QPaintEvent *event);
    void draw_coordinates(QPaintEvent *event);
    void draw_effects(QPaintEvent *event);
    void draw_pieces(QPaintEvent *event);
    void draw_shapes(QPaintEvent *event);
    void draw_promotion(QPaintEvent *event);

    void draw_arrow(QPainter *painter, QPointF start, QPointF end, float line_width, float arrow_width,
                    float arrow_height, QColor color);

    void repaint_pieces();
    void recalculate_figurines();

    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    db::Move make_move(db::Square from, db::Square to, db::Piece promotion);
    void start_animation(const db::Position &start, const db::Position &target);
    void start_move_animation(const db::Move &move);
    void start_move_undo_animation(const db::Move &move);

    void queue_move_animation(const db::MoveId id, const db::Move &move, bool undo = false);
    void process_animation_queue();

    std::vector<std::tuple<db::MoveId, db::Move, bool>> m_move_animation_queue;

    db::Board m_board;

private:
    struct Arrow
    {
        db::Square from;
        db::Square to;
        QColor color;
    };

    std::map<db::Square, QColor> m_circles; // TODO: switch to std::vector
    std::vector<Arrow> m_arrows;

    QColor m_shape_green;
    QColor m_shape_red;
    QColor m_shape_blue;
    QColor m_shape_yellow;
    Qt::KeyboardModifiers m_modifiers;

    bool m_fast_last_move;

signals:
    void move_made(const db::Move &move, bool animated = true);
    void fen_changed(const std::string &fen);
    void current_move(const db::MoveId id);
    void forward();
    void back();
    void get_prev_move();
};
