/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdhdr.h"
#include "object.h"
#include "../common.h"
#include "lowlevelstr.h"
#include "strutils.h"
#include "ObjectManager.h"

int Object::LoadFrom(FILE *f)
{
	StringList temp;
	return temp.loadFromFile(f) && LoadFromList(temp);
}

int Object::SaveTo(FILE *f)
{
	StringList temp;
	SaveToList(temp);
	return temp.saveToFile(f);
}

Property& Object::Property(const String& v)
{
	int j=_class->PropertyNames.Find(v);
	if (j!=-1)
		return properties[_class->PropertyNames.Values(j)];
	else 
		return undef_property;
}

int Object::Load(const String& fn)
{
	StringList temp;
	return temp.loadFromFile(fn) && LoadFromList(temp);
}

int Object::Save(const String& fn)
{
	StringList temp;
	SaveToList(temp);
	return temp.saveToFile(fn);
}

Object& Object::Child(const String& v)
{
	for(int i = 0; i < childs.Count(); ++i)
	{
		if(childs[i]->Name() == v)
			return Child(i);
	}
	FWError(Format(L"Request to undefined object %s", v.ptr()));
	return *((Object*)NULL);
}

void Object::ClearChilds()
{
	for(int i = 0; i < childs.Count(); ++i)
	{
		delete childs[i];
	}
	childs.Clear();
}


int Object::LoadFromList(StringList &list, int start)
{
	ClearChilds();
	BeforeLoad();

	int i=start, res=list.Count()-1;
	while (i<list.Count())
	{
		String line=list[i].trim();
		if (line=="end") {
			res=i;
			break;
		} else if (!line.ncmp("object", 6)) {
			int p=line.find(' ');
			int p1=line.find(':');
			if (p!=-1 && p1!=-1 && p<p1) {
				String pname=line.substr(p+1, p1-p-1).trim();
				String ptype=line.substr(p1+1).trim();
				Object *obj=objectManager->Create(ptype, pname, this);
				if (obj) {
					i=obj->LoadFromList(list, i+1);
				} else {
					FWError(Format(L"Object type %s is undefined", ptype.ptr()));
				}
			}
		} else  {
			int p=line.find('=');
			if (p!=-1)	{
				String pline=line.substr(0, p).trim();
				String pval=line.substr(p+1).trim().trimquotes();
				(*this)(pline)=pval;
			}
		}
		i++;
	}

	AfterLoad();
	loaded_properties = properties;
	return res;
}

void Object::SaveToList(StringList &list, int clear, int level)
{
	if (clear) list.Clear();
	String pfx;
	for (int j=0; j<level; j++) pfx+="  ";
	for (int i=0; i<_class->PropertyNames.Count(); i++)
	{
		list.Add(pfx+_class->PropertyNames[i]+"="
			+properties[_class->PropertyNames.Values(i)]);
	}
	for (int i=0; i<childs.Count(); i++)
	{
		list.Add(pfx+"object "+childs[i]->Name()+": "+childs[i]->Type());
		childs[i]->SaveToList(list, 0, level+1);
		list.Add(pfx+"end");
	}
}

void Object::ReloadProperties()
{
	properties = loaded_properties;
}

void Object::ReloadPropertiesRecursive()
{
	ReloadProperties();
	for (int i=0; i<childs.Count(); i++)
		childs[i]->ReloadPropertiesRecursive();
}

