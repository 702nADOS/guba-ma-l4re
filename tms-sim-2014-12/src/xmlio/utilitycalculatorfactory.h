/**
 * $Id: utilitycalculatorfactory.h 501 2014-05-07 14:16:17Z klugeflo $
 * @file utilitycalculatorfactory.h
 * @brief Creation of utility calculators from XML
 * @authors Peter Ittner, Florian Kluge <kluge@informatik.uni-augsburg.de>
 */

#ifndef XMLIO_UTILITYCALCULATORFACTORY_H
#define XMLIO_UTILITYCALCULATORFACTORY_H 1

/**
 * Includes
 */
#include <string>
#include <map>

#include <core/scobjects.h>
#include <xmlio/ielementbuilder.h>
#include <xmlio/ielementfactory.h>

namespace tmssim {

/**
 * Creates UtilityCalculator objects from a XML document
 * @todo Make this class a singleton.
 * @todo Load builders from dynamic library
 */
class UtilityCalculatorFactory: public IElementFactory<UtilityCalculator> {

	/**
	 * Member
	 */
private:
	/**
	 * Container that maps strings (XML tag names) to builder-objects,
	 * which are able to construct the prober object
	 */
	std::map<std::string, IElementBuilder<UtilityCalculator>*> mBuilder;

public:
	/**
	 * C'tor
	 * Initializes the map
	 */
	UtilityCalculatorFactory();

	/**
	 * Checks if the factory is able to construct a UtilityCalculator at the current document position
	 * @param doc The XML document
	 * @param cur The current document position
	 * @return true, if this factory can construct a UtilityCalculator, otherwise false
	 */
	bool accept(xmlDocPtr doc, xmlNodePtr cur);

	/**
	 * Calls the build method of the proper builder to construct a UtilityCalculator for the given position within
	 * the XML-document
	 * @param doc The XML-document
	 * @param cur The current position within the document
	 * @return A pointer to a UtilityCalculator object, in case of an Error NULL
	 */
	UtilityCalculator* getElement(xmlDocPtr doc, xmlNodePtr cur);

	~UtilityCalculatorFactory();
};

} // NS tmssim

#endif /* !XMLIO_UTILITYCALCULATORFACTORY_H */
