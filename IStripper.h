/** ======== stripper_mm ========
 *  Copyright (C) 2005-2006 David "BAILOPAN" Anderson
 *  No warranties of any kind.
 *  Based on the original concept of Stripper2 by botman
 *
 *  License: see LICENSE.TXT
 *  ============================
 */

#ifndef _INCLUDE_STRIPPER_INTERFACE_H
#define _INCLUDE_STRIPPER_INTERFACE_H

#define	STRIPPER_INTERFACE		"IStripper"

class IStripperListener
{
public:
	//Called when it's safe to AddMapEntityFilter
	virtual void OnMapInitialize(const char *mapname) =0;
	//Called when the Stripper plugin is unloading
	virtual void Unloading()
	{
	};
};

class IStripper
{
public:
	/**
	 * Adds a listener for maps
	 */
	virtual void AddMapListener(IStripperListener *pListener) =0;

	/**
	 * Removes a listener for maps
	 */
	virtual void RemoveMapListener(IStripperListener *pListener) =0;

	/** 
	 * Adds a map entity filter file to the map's
	 *  entity list.  You must call this during
	 *  IStripperListener::OnMapInitialize()
	 */
	virtual void AddMapEntityFilter(const char *file) =0;
};

#endif //_INCLUDE_STRIPPER_INTERFACE_H
