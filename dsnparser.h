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
#include <mutex>
#include <vector>

#define _DSNBEGIN_ namespace DSN{
#define _DSNEND_     };

_DSNBEGIN_
using namespace std;

struct pin
{
	std::string m_name;
	std::string m_form;
	double m_x;
	double m_y;
	double m_angle;
};

struct component
{
	std::string m_name;
	std::map<std::string, pin> m_pin_map;
};

struct instance
{
	std::string m_name;
	std::string m_comp;
	std::string m_side;
	double m_x;
	double m_y;
	double m_angle;
};

struct rule
{
	double m_radius;
	double m_gap;
};

struct circuit
{
	std::string m_via;
	rule m_rule;
};

struct padstack
{
	points_2d m_shape;
	rule m_rule;
};


struct tree
{
	std::string m_value;
	std::vector <tree> m_branches;
};

class DsnParser
{
public:
    static DsnParser& getInstance(const std::string& fileName);
    
private:    
    tree pcbTree;

private:
    //read input till given byte appears
    auto read_until(std::istream &in, char c);
    //read whitespace
    auto read_whitespace(std::istream &in);
    auto read_node_name(std::istream &in);
    auto read_string(std::istream &in);
    auto read_quoted_string(std::istream &in);
    tree read_tree(std::istream &in);
    tree *search_tree(tree &t, const char *s);
    void print_tree(const tree &t, int indent);
    void ss_reset(std::stringstream &ss, const std::string &s);
    void get_value(std::stringstream &ss, std::vector<tree>::iterator t, int &x);
    void get_value(std::stringstream &ss, std::vector<tree>::iterator t, double &x);
    void get_2d(std::stringstream &ss, std::vector<tree>::iterator t, double &x, double &y);
    void get_rect(std::stringstream &ss, std::vector<tree>::iterator t, double &x1, double &y1, double &x2, double &y2);
private:
    DsnParser(const std::string& filename);
    static void init(const std::string& filename);
    static DsnParser* dsnparser;   
    static std::once_flag initFlag_; 

};

std::once_flag DsnParser::initFlag_;
DsnParser* DsnParser::dsnparser = nullptr;


_DSNEND_