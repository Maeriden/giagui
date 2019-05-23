#ifndef GIAGUI_GEOVALUEVALIDATOR_H
#define GIAGUI_GEOVALUEVALIDATOR_H

#include <QDoubleValidator>


class GeoValueValidator : public QDoubleValidator
{
public:
	explicit GeoValueValidator(int decimals, QObject* parent = nullptr);
	explicit GeoValueValidator(double min, double max, int decimals, QObject* parent = nullptr);
	QValidator::State validate(QString &input, int &pos) const override;
};

#endif // GIAGUI_GEOVALUEVALIDATOR_H
