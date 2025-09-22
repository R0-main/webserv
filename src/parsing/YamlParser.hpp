/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   YamlParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 12:15:48 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 14:30:56 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../utils/Logger.hpp"
#include "../utils/String.hpp"
#include <iostream>
#include <vector>
#include <map>

typedef enum
{
	BOOLEAN,
	INT,
	STRING,
	ARRAY,
	DICT,
	__TYPE_COUNT__
} e_value_type;

class YamlDict;

class IYamlNode
{
	protected:
		e_value_type _type;

	public:
		YamlDict	*parent;
		std::size_t indent_level;

		virtual ~IYamlNode() {}

		virtual bool isInt() const { return this->_type == INT; };
		virtual bool isString() const { return this->_type == STRING; };
		virtual bool isBoolean() const { return this->_type == BOOLEAN; };
		virtual bool isArray() const { return this->_type == ARRAY; };

		virtual bool isDict() const {return this->_type == DICT;};

		virtual void print(Logger &logger, std::size_t offset = 1) const throw () = 0;
};

class YamlDict : public IYamlNode
{
	public:
		typedef std::map<String, IYamlNode*>		t_map;

	private:

	t_map									_map;

	public:
		std::size_t child_indent;

		bool has(String		key) const throw()
		{
			t_map::const_iterator it = this->_map.find(key);
			if (it != this->_map.end() && it->second != NULL) {
				return true;
			}
			return false;
		}

		IYamlNode *get(String	key)
		{
			if (!this->has(key))
			{
				_Logger.error("Key doesn't exist !");
				throw std::exception();
			}
			return this->_map[key];
		}

		void set(String	key, IYamlNode *node)
		{
			this->_map[key] = node;
		}

		void print(Logger &logger, std::size_t offset = 1) const throw()
		{
			for (t_map::const_iterator i = this->_map.begin(); i != this->_map.end(); i++)
			{
				logger << std::string(offset, '	');
				logger << (*i).first;
				logger << " -> ";
				logger << (*i).second;
				logger << "\n";
				(*i).second->print(logger, offset + 1);
			}
		}

		t_map::const_iterator begin(void) const throw()
		{
			return this->_map.begin();
		}

		t_map::const_iterator end(void) const throw()
		{
			return this->_map.end();
		}

		YamlDict(YamlDict *parent = NULL)
		{
			this->indent_level = 0;
			this->child_indent = -1;
			this->_type = DICT;
			this->parent = parent;
		}

		~YamlDict() {
			for (t_map::iterator i = this->_map.begin(); i != this->_map.end(); i++)
			{
				if ((*i).second)
					delete (*i).second;
			}
		};
};

template <typename T>
class YamlValue : public IYamlNode
{
	private:

	public:
		T _value;

		YamlValue(YamlDict *parent, T value, e_value_type type) : _value(value)
		{
			this->indent_level = 0;
			this->parent = parent;
			this->_type = type;
		}

		void print(Logger &logger, std::size_t offset = 1) const throw ()
		{
			logger << std::string(offset, '	');
			logger << "Value :	" << _value << "\n";
			logger << std::string(offset, '	');
			logger << "Type :	";
			switch (this->_type)
			{
				case INT:
					logger << "INT";
					break;
				case STRING:
					logger << "STRING";
					break;

				default:
					logger << "NOT DEFINED";
					break;
			}
			logger << "\n";
		}
};

class YamlParser
{
	private:
		String							_config_file_path;
		Logger							_logger;

		std::ifstream					_file;

		YamlDict						*_start;

		std::vector<std::pair<std::size_t, String> >				_tokens;

	public:

		YamlParser(String path) : _config_file_path(path), _logger("YamlParser", Colors::CYAN)
		{
			this->_logger.info("Initialisation ...");

			if (!this->_config_file_path.endsWith(".yml") && !this->_config_file_path.endsWith(".yaml"))
			{
				this->_logger.error("Wrong file extension !");
				return;
			}
			this->_start = new YamlDict();
			this->_file.open(_config_file_path.c_str());
			this->parse();
		}

		void interpret_tokens(void)
		{
			YamlDict *dict = this->_start;

			for (std::vector<std::pair<std::size_t, String> >::iterator i = this->_tokens.begin(); i != this->_tokens.end(); i++)
			{
				this->_logger.log((*i).first);
				this->_logger.log((*i).second);
			}

			for (std::vector<std::pair<std::size_t, String> >::iterator i = this->_tokens.begin(); i != this->_tokens.end(); i++)
			{
				std::size_t indent = (*i).first;
				String line = (*i).second;

				if (line.count(": ") > 1)
				{
					this->_logger.error("Too many : !");
					break;
				}

				if (indent % 2 != 0)
				{
					this->_logger.error("Invalid Indentation !");
					break;
				}
				if (dict->indent_level == indent && dict->parent)
				{
					if (dict->child_indent == -1)
					{
						this->_logger.error("Missing Value");
						break;
					}
					dict = dict->parent;
				}
				else if (dict->parent && dict->child_indent != indent && dict->child_indent != -1 && indent != 0)
				{
					this->_logger.error("Invalid Indent");
					break;
				}
				else
				{
					if (indent == 0)
					{
						dict = this->_start;
					}
					else if (dict->child_indent != -1 && indent != dict->child_indent)
					{
						this->_logger.error("Invalid Indent");
						break;
					}
					dict->child_indent = indent;
				}
				std::size_t column = line.find(":");
				if (column != std::string::npos)
				{
					if (column == line.size() - 1)
					{
						YamlDict *created = new YamlDict(dict);
						created->indent_level = indent;
						String key = line.substr(0, column);
						key.trimQuotes();
						dict->set(key, created);
						dict = created;
					}
					else
					{
						String key = line.substr(0, column);
						key.trimQuotes();
						String value = line.substr(column + 1, line.size());
						value.trim();

						IYamlNode *valueNode;

						if (value.isInt())
						{
							YamlValue<int> *value_ = new YamlValue<int>(dict, value.toInt(), INT);
							valueNode = value_;
						}
						else
						{
							value.trimQuotes();
							YamlValue<String> *value_ = new YamlValue<String>(dict, value, STRING);
							valueNode = value_;
						}

						valueNode->indent_level = indent;
						dict->set(line.substr(0, column), valueNode);
					}
				}
				else
				{
					std::size_t dict = line.startsWith("- ");
				}
			}

			this->_start->print(this->_logger);
		}

		void parse(void)
		{
			std::vector<String> lines;
			String line;

			while (getline(this->_file, line))
			{
				if (line.empty() || line.startsWith("#"))
					continue;
				lines.push_back(line);
			}

			for (std::vector<String>::iterator i = lines.begin(); i != lines.end(); i++)
			{
				(*i).trimEnd();
				std::size_t indent = (*i).countCharUntilAnOther(0, ' ');
				(*i).trim();
				std::size_t comment = (*i).find("#");
				if (comment != std::string::npos)
					(*i).erase(comment, (*i).size());
				if ((*i).empty())
					continue;
				this->_tokens.push_back(std::make_pair(indent, *i));
			}

			this->interpret_tokens();
		}

		YamlDict *get() throw()
		{
			return this->_start;
		}

		~YamlParser()
		{
			delete this->_start;
		}

};

