/*
    This file is part of the FElt finite element analysis package.
    Copyright (C) 1993-2000 Jason I. Gobat and Darren C. Atkinson

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/************************************************************************
 * File:	ElementList.h						*
 *									*
 * Description:	This file contains the function and type declarations	*
 *		for the element list box.				*
 ************************************************************************/

# ifndef _ElementList_h
# define _ElementList_h
# include "fe.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*----------------------------------------------------------------------*/

typedef struct element_list *ElementList;

ElementList ElementListCreate (Widget parent, String name, String title);

void ElementListPopup (ElementList elementl);

void ElementListSetupNames (ElementList elementl);

void ElementListPopdown (ElementList elementl);

void ElementListSet (ElementList elementl, Definition definition);

Definition ElementListDefinition (ElementList elementl);

String ElementListName (ElementList elementl);

/*----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif 

# endif /* _ElementList_h */
