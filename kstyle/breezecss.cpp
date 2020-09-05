#include "breezecss.h"

void CSSParser() {
    // <integer>
    // https://developer.mozilla.org/en-US/docs/Web/CSS/integer
    let integer = Map<qint64>(
        [](QString sign, QList<QString> in) -> qint64 {
            var parsedInteger = QStringList(in).join("").toInt();
            if (sign == "-") {
                parsedInteger *= -1;
            }
            return parsedInteger;
        },
        ParseString("+")->Or(ParseString("-"))->Or(ParseString("")),
        ParseToken([](QChar ch) { return ch.isDigit(); })->Many()
    );
    let floating = Map<qreal>(
        [](QString sign, QList<QString> in) -> qreal {
            var parsedFloat = QStringList(in).join("").toDouble();
            if (sign == "-") {
                parsedFloat *= -1;
            }
            return parsedFloat;
        },
        ParseString("+")->Or(ParseString("-"))->Or(ParseString("")),
        ParseToken([](QChar ch) { return ch.isDigit() || ch == '.'; })->Many()
    );
    let scientific = Map<qreal>(
        [](qreal first, QString, qint64 second) -> qreal {
            return first * qPow(10, second);
        },
        floating, ParseString("e"), integer
    );

    // <number>
    // https://developer.mozilla.org/en-US/docs/Web/CSS/number
    let number = integer->Or(floating->Or(scientific));

    let lengthUnit = ParseString({"cap", "ch", "em", "ex", "ic", "lh", "rem", "rlh", "vh", "vw", "vi", "vb", "vmin", "vmax", "px", "cm", "mm", "Q", "in", "pc", "pt"});

    let length = number->ThenAlso(lengthUnit);
}