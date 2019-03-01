
#include "DoubleValidator.hpp"



DoubleValidator::DoubleValidator(double min, double max, int decimals, QObject* parent ) : QDoubleValidator(min, max, decimals, parent) {}

DoubleValidator::DoubleValidator(int decimals, QObject* parent ) : QDoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, decimals, parent) {}

QValidator::State DoubleValidator::validate(QString& input, int& pos) const
{
	if(input.isEmpty())
		return QValidator::Acceptable;
	return QDoubleValidator::validate(input, pos);
}