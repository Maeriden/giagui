#include "GeoValueValidator.hpp"

#include "MapUtils.hpp"


GeoValueValidator::GeoValueValidator(double min, double max, int decimals, QObject* parent ) : QDoubleValidator(min, max, decimals, parent){}


GeoValueValidator::GeoValueValidator(int decimals, QObject* parent ) : QDoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, decimals, parent) {}


QValidator::State GeoValueValidator::validate(QString& input, int& pos) const
{
	if(input.isEmpty())
		return QValidator::Acceptable;
	if(input == QString::fromUtf8(UI_MULTIPLE_GEOVALUES_STRING))
		return QValidator::Acceptable;
	return QDoubleValidator::validate(input, pos);
}
