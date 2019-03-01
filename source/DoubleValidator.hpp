
#ifndef DOUBLEVALIDATOR_H
#define DOUBLEVALIDATOR_H


#include <QtGui/QDoubleValidator>
#include "map.hpp"


class DoubleValidator : public QDoubleValidator
{
	public:
		explicit DoubleValidator(int decimals, QObject* parent = nullptr);
		
		explicit DoubleValidator(double min, double max, int decimals, QObject* parent = nullptr);
		
		QValidator::State validate(QString &input, int &pos) const override;
	
};

#endif // DOUBLEVALIDATOR_H
