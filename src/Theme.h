#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QObject>

// Single source of truth for the palette. Exposed to QML as the `Theme`
// context property, so every component can use Theme.accent and friends
// without importing anything.
class Theme : public QObject {
    Q_OBJECT

    // Surfaces, from the deepest layer (window) to the most raised (hover).
    Q_PROPERTY(QColor bg READ bg CONSTANT)
    Q_PROPERTY(QColor surface READ surface CONSTANT)
    Q_PROPERTY(QColor surfaceAlt READ surfaceAlt CONSTANT)
    Q_PROPERTY(QColor surfaceHover READ surfaceHover CONSTANT)
    Q_PROPERTY(QColor surfaceSunken READ surfaceSunken CONSTANT)

    Q_PROPERTY(QColor border READ border CONSTANT)
    Q_PROPERTY(QColor borderSoft READ borderSoft CONSTANT)

    Q_PROPERTY(QColor text READ text CONSTANT)
    Q_PROPERTY(QColor textMuted READ textMuted CONSTANT)
    Q_PROPERTY(QColor textFaint READ textFaint CONSTANT)

    Q_PROPERTY(QColor accent READ accent CONSTANT)
    Q_PROPERTY(QColor accentHover READ accentHover CONSTANT)
    Q_PROPERTY(QColor accentPressed READ accentPressed CONSTANT)
    Q_PROPERTY(QColor onAccent READ onAccent CONSTANT)
    Q_PROPERTY(QColor accentSoft READ accentSoft CONSTANT)

    Q_PROPERTY(QColor success READ success CONSTANT)
    Q_PROPERTY(QColor warning READ warning CONSTANT)
    Q_PROPERTY(QColor danger READ danger CONSTANT)
    Q_PROPERTY(QColor dangerHover READ dangerHover CONSTANT)
    // Dimming layer behind modal dialogs.
    Q_PROPERTY(QColor scrim READ scrim CONSTANT)

    Q_PROPERTY(int radiusSm READ radiusSm CONSTANT)
    Q_PROPERTY(int radiusMd READ radiusMd CONSTANT)
    Q_PROPERTY(int radiusLg READ radiusLg CONSTANT)
    Q_PROPERTY(int gapSm READ gapSm CONSTANT)
    Q_PROPERTY(int gapMd READ gapMd CONSTANT)
    Q_PROPERTY(int gapLg READ gapLg CONSTANT)

public:
    explicit Theme(QObject *parent = nullptr) : QObject(parent) {}

    QColor bg() const { return QColor("#14171c"); }
    QColor surface() const { return QColor("#1b1f26"); }
    QColor surfaceAlt() const { return QColor("#232932"); }
    QColor surfaceHover() const { return QColor("#2c333e"); }
    QColor surfaceSunken() const { return QColor("#101318"); }

    QColor border() const { return QColor("#343c49"); }
    QColor borderSoft() const { return QColor("#262c35"); }

    QColor text() const { return QColor("#e7ebf1"); }
    QColor textMuted() const { return QColor("#99a3b2"); }
    QColor textFaint() const { return QColor("#6c7686"); }

    QColor accent() const { return QColor("#5b9df9"); }
    QColor accentHover() const { return QColor("#74aefb"); }
    QColor accentPressed() const { return QColor("#4886dc"); }
    QColor onAccent() const { return QColor("#0e141b"); }
    QColor accentSoft() const { return QColor("#1e3550"); }

    QColor success() const { return QColor("#3fb950"); }
    QColor warning() const { return QColor("#d29922"); }
    QColor danger() const { return QColor("#e5534b"); }
    QColor dangerHover() const { return QColor("#f06a62"); }
    QColor scrim() const { return QColor(0, 0, 0, 170); }

    int radiusSm() const { return 8; }
    int radiusMd() const { return 10; }
    int radiusLg() const { return 14; }
    int gapSm() const { return 8; }
    int gapMd() const { return 12; }
    int gapLg() const { return 16; }
};

#endif // THEME_H
