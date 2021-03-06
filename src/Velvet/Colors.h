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
 * File:	Colors.h						*
 *									*
 * Description:	This file contains the public function and type		*
 *		declarations for the colors dialog box.			*
 ************************************************************************/

# ifndef _Colors_h
# define _Colors_h

#include "problem.h"

/*----------------------------------------------------------------------*/

typedef struct colors_dialog *ColorsDialog;

ColorsDialog ColorsDialogCreate (Widget parent, String name, String title);

void ColorsDialogUpdateConstraintList (ColorsDialog colorsd, Problem::ConstraintSet *tree, Boolean deleted);

void ColorsDialogUpdateMaterialList (ColorsDialog colorsd, Problem::MaterialSet *tree, Boolean deleted);

void ColorsDialogUpdateDistributedList (ColorsDialog colorsd, Problem::DistributedSet *tree, Boolean deleted);

void ColorsDialogUpdateForcesList (ColorsDialog colorsd, Problem::ForceSet *tree, Boolean deleted);
     
void ColorsDialogPopup (ColorsDialog colorsd);

/*----------------------------------------------------------------------*/

# endif /* _Colors_h */
