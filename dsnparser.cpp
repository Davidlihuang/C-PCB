/*
    C-PCB
    Copyright (C) 2015 Chris Hinsley
	chris (dot) hinsley (at) gmail (dot) com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "router.h"
#include "math.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "dsnparser.h"


using namespace DSN;
using namespace std;

std::once_flag DsnParser::initFlag_;
DsnParser* DsnParser::dsnparser = nullptr;

DsnParser::DsnParser(const std::string& filename)
{
    //parser dsn file to tree
	this->infile.open(filename, std::istream::in);
   if(this->infile.is_open())
   {
		this->read_tree(this->infile);
   }
   else{
	    std::cout << "reading .dsn file failed!" <<std::endl;
   }
	
}
tree DsnParser::get_tree()
{
	return this->pcbTree;
}
void DsnParser::init(const std::string& filename)
{
    dsnparser = new DsnParser(filename);    
}

DsnParser& DsnParser::getInstance(const std::string& fileName)        
{
    std::call_once(initFlag_, &DsnParser::init, fileName);
    return *dsnparser;
}


// std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
// {
// 	std::stringstream ss(s);
// 	std::string item;
// 	while (std::getline(ss, item, delim))
// 	{
// 		elems.push_back(item);
// 	}
// 	return elems;
// }

// std::vector<std::string> split(const std::string &s, char delim)
// {
// 	std::vector<std::string> elems;
// 	split(s, delim, elems);
// 	return elems;
// }

// auto shape_to_cords(const points_2d &shape, double a1, double a2)
// {
// 	auto cords = points_2d{};
// 	auto rads = fmod(a1+a2, 2*M_PI);
// 	auto s = sin(rads);
// 	auto c = cos(rads);
// 	for (auto &p : shape)
// 	{
// 		auto px = double(c*p.m_x - s*p.m_y);
// 		auto py = double(s*p.m_x + c*p.m_y);
// 		cords.push_back(point_2d{px, py});
// 	}
// 	return cords;
// }

//read input till given byte appears
auto DsnParser::read_until(std::istream &in, char c)
{
	char input;
	while (in.get(input))
	{
		if (input == c) return false;
	}
	return true;
}

//read whitespace
auto DsnParser::read_whitespace(std::istream &in)
{
	for (;;)
	{
		auto b = in.peek();
		if (b != '\t' && b != '\n' && b != '\r' && b != ' ') break;
		char c;
		in.get(c);
	}
}

auto DsnParser::read_node_name(std::istream &in)
{
	std::string s;
	for (;;)
	{
		auto b = in.peek();
		if (b == '\t' || b == '\n' || b == '\r' || b == ' ' || b == ')') break;
		char c;
		in.get(c);
		s.push_back(c);
	}
	return s;
}

auto DsnParser::read_string(std::istream &in)
{
	std::string s;
	for (;;)
	{
		auto b = in.peek();
		if (b == '\t' || b == '\n' || b == '\r' || b == ' ' || b == ')') break;
		char c;
		in.get(c);
		s.push_back(c);
	}
	return tree{s, {}};
}

auto DsnParser::read_quoted_string(std::istream &in)
{
	std::string s;
	for (;;)
	{
		auto b = in.peek();
		if (b == '"') break;
		char c;
		in.get(c);
		s.push_back(c);
	}
	return tree{s, {}};
}

tree DsnParser::read_tree(std::istream &in)
{
	read_until(in, '(');
	read_whitespace(in);
	std::string nodeName = read_node_name(in);
	auto t = tree{nodeName, {}};
	for (;;)
	{
		read_whitespace(in);
		auto b = in.peek();
		char c;
		if (b == EOF) break;
		if (b == ')') // return condition
		{
			in.get(c);
			break;
		}
		if (b == '(')
		{
			t.m_branches.push_back(read_tree(in));
			continue;
		}
		if (b == '"')
		{
			if(nodeName != "string_quote")
			{
				in.get(c);
				t.m_branches.push_back(read_quoted_string(in));
				in.get(c);
				continue;
			}
			else
			{
				in.get(c);
				std::string str;
				str.push_back(b);
				t.m_branches.push_back(tree{str, {}});
				continue;
			}
		}
		t.m_branches.push_back(read_string(in));
	}
	this->pcbTree = t;
	return  t;
}

tree* DsnParser::search_tree(tree &t, const char *s)
{
	if (t.m_value == s) return &t;
	for (auto &ct : t.m_branches)
	{
		auto st = search_tree(ct, s);
		if (st != nullptr) return st;
	}
	return nullptr;
}

void DsnParser::print_tree(const tree &t, int indent)
{
	if (!t.m_value.empty())
	{
		for (auto i = 0; i < indent; ++i) std::cout << "  ";
		std::cout << t.m_value << '\n';
	}
	for (auto &ct : t.m_branches) print_tree(ct, indent+1);
}

void DsnParser::ss_reset(std::stringstream &ss, const std::string &s)
{
	ss.str(s);
	ss.clear();
}

void DsnParser::get_value(std::stringstream &ss, std::vector<tree>::iterator t, int &x)
{
	ss_reset(ss, t->m_value);
	ss >> x;
}

void DsnParser::get_value(std::stringstream &ss, std::vector<tree>::iterator t, double &x)
{
	ss_reset(ss, t->m_value);
	ss >> x;
}

void DsnParser::get_2d(std::stringstream &ss, std::vector<tree>::iterator t, double &x, double &y)
{
	get_value(ss, t, x);
	get_value(ss, t + 1, y);
}

void DsnParser::get_rect(std::stringstream &ss, std::vector<tree>::iterator t, double &x1, double &y1, double &x2, double &y2)
{
	get_2d(ss, t, x1, y1);
	get_2d(ss, t + 2, x2, y2);
}
