/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef NS_LISTDOUBLE_H
#define NS_LISTDOUBLE_H

#include "attribute.h"
#include "attribute-helper.h"
#include <stdint.h>
#include <limits>

// Alejandro
#include <vector>

/**
 * \file
 * \ingroup attribute_Double
 * ns3::DoubleValue attribute value declarations and template implementations.
 */

namespace ns3 {

//  Additional docs for class DoubleValue:
/**
 * This class can be used to hold variables of floating point type
 * such as 'double' or 'float'. The internal format is 'double'.
 */
ATTRIBUTE_VALUE_DEFINE_WITH_NAME (std::vector<double>, ListDouble);
ATTRIBUTE_ACCESSOR_DEFINE (ListDouble);
ATTRIBUTE_CHECKER_DEFINE (ListDouble);

} // namespace ns3

#endif /* NS_DOUBLE_H */
