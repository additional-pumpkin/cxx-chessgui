#pragma once
#include <QKeyEvent>
#include <QWidget>

#include "game.hxx"
class NotationView : public QWidget
{
    struct MoveBox
    {
        db::MoveNode node;
        QRectF hitbox;
    };

    Q_OBJECT
public:
    explicit NotationView(QWidget *parent = nullptr);

    void forward();
    void back();

    void add_move(const db::Move &move, bool animated = true);
    void set_current_move(const db::MoveId id);
    void get_prev_move()
    {
        size_t index = m_game.find_move(m_current_move_id);
        if (index == 0) {
            emit prev_move(db::Move());
            return;
        }
        auto moves = m_game.get_moves();
        if (moves[index - 1].variation_id == moves[index].variation_id) {
            emit prev_move(moves[index - 1].move);
            return;
        }

        for (auto it = moves.rend() + (signed)index + 1; it != moves.rend(); --it) {
            if (it->variation_level == moves[index].variation_level - 1) {
                emit prev_move(it->move);
                return;
            }
        }
    }

private:
    void recalculate_moveboxes();

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    db::Game m_game;

    int m_move_height;
    db::MoveId m_current_move_id;
    size_t m_current_move_index;
    std::vector<MoveBox> m_move_boxes;
signals:
    void forward_move(const db::MoveId id, const db::Move &move);
    void back_move(const db::MoveId id, const db::Move &move);
    void move_added(const db::MoveId id, const db::Move &move, bool animated = true);
    void prev_move(const db::Move &move);
    void text_changed(const std::string text);
};
