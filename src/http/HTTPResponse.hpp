/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 12:14:12 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/11 14:06:03 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "../utils/SystemFiles.hpp"
#include "../utils/String.hpp"
#include "HTTPProtocol.hpp"
#include "../Server.hpp"
#include <map>
#include <netinet/in.h>
#include <unistd.h>
#include <filesystem>
#include "dirent.h"

#define STYLE "<style> :root{ --fg:#e9e7ff; } *{box-sizing:border-box} html,body{height:100%} body{ margin:0; padding:clamp(16px,3vw,32px); background-color:#07051a; /* deep black-violet base */ color:var(--fg); font:500 16px/1.4 ui-sans-serif,system-ui,-apple-system,Segoe UI,Roboto,Inter,\"Helvetica Neue\",Arial,\"Noto Sans\",\"Apple Color Emoji\",\"Segoe UI Emoji\"; display:grid; align-content:start; gap:14px; } /* Blurred purple/black gradient backdrop */ body::before{ content:\"\"; position:fixed; inset:-80px; background: radial-gradient(600px 400px at 10% 10%, rgba(167,139,250,.6), transparent 60%), radial-gradient(800px 500px at 90% 20%, rgba(99,102,241,.5), transparent 65%), radial-gradient(700px 500px at 50% 100%, rgba(168,85,247,.35), transparent 60%), linear-gradient(180deg, #0a0614 0%, #0b0820 45%, #05030d 100%); filter: blur(80px) saturate(120%); z-index:-1; transform: scale(1.12); } br{display:none} /* turn the list into cards */ a{ display:flex; align-items:center; gap:12px; padding:14px 16px; border-radius:16px; background:rgba(11,8,32,.55); border:1px solid rgba(167,139,250,.18); backdrop-filter: blur(12px); -webkit-backdrop-filter: blur(12px); text-decoration:none; color:inherit; min-width:0; white-space:nowrap;overflow:hidden;text-overflow:ellipsis; box-shadow:0 1px 1px rgba(0,0,0,.15),0 10px 24px rgba(0,0,0,.35); transition:transform .12s ease,box-shadow .2s ease,border-color .2s ease,background .2s ease; } a:hover{ transform:translateY(-2px); border-color:rgba(167,139,250,.45); box-shadow:0 14px 32px rgba(0,0,0,.45); background:rgba(21,16,48,.65); } a:focus-visible{ outline:none; box-shadow:0 0 0 3px rgba(167,139,250,.35),0 10px 24px rgba(0,0,0,.35); } a::first-letter{ /* emoji icon boost (📁/📄) */ font-size:1.35em; line-height:0; } a::after{ content:\"↗\"; margin-left:auto; opacity:.25; transition:opacity .2s ease,transform .12s ease; } a:hover::after{opacity:.7;transform:translateX(2px)} @supports (width: min(100%, 640px)){ body{grid-template-columns:repeat(auto-fill,minmax(min(100%,420px),1fr))} } /* Scrollbar polish (WebKit) */ ::-webkit-scrollbar{width:10px;height:10px} ::-webkit-scrollbar-thumb{ background:rgba(233,231,255,.25); border-radius:999px;border:2px solid transparent;background-clip:content-box } ::-webkit-scrollbar-track{background:transparent} </style>"
#define TITLE "<title>My Page Title</title>"
#define ICON "<link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">"

class HTTPResponse : public HTTPProtocol
{
	public:

		String			status;
		unsigned int	status_code;

		HTTPResponse() : HTTPProtocol(), status("OK"), status_code(200)
		{

		}

		~HTTPResponse()
		{

		}

		void send(fd_t client_fd)
		{
			String request("HTTP/1.1 " + String::fromInt(status_code) + String(" ") + status + String(CRLF));

			if (!this->body.empty() && this->hasHeader("content-length") == false)
			{
				this->setHeader("content-length", String::fromInt(this->body.length()));
			}
			else if (this->body.empty() && this->hasHeader("content-length") == false)
			{
				this->body = "<html><body><h1>" + String::fromInt(this->status_code) + " - " + this->status + "</h1></body></html>";
				this->setHeader("content-length", "80");
			}

			// Send headers
			request.append(this->generate_headers());
			// request.append(CRLF); // Empty line between headers and body
			write(client_fd, request.c_str(), request.length());

			// Send body separately for binary data safety
			if (!this->body.empty()) {
				write(client_fd, this->body.data(), this->body.length());
			}
		}

		void setupFile(String file_path, String request_target) throw()
		{
			if (SystemFiles::isDirectory(file_path))
			{
				this->body = "<html><head><meta charset=\"UTF-8\">" STYLE ICON TITLE "</head><body>";
				DIR *dir;
				struct dirent *ent;

				_Logger.debug(request_target);
				_Logger.debug(file_path);

				if ((dir = opendir(file_path.c_str())) != NULL) {

					while ((ent = readdir (dir)) != NULL) {
						if (String(ent->d_name) == ".") continue;
						this->body += "<a href=" + (request_target == "/" ? " " : request_target ) + (request_target.endsWith("/") ? "" : "/") + String(ent->d_name) + ">" + (ent->d_type == DT_DIR ? "📁" : "📄") +  " ./" + ent->d_name + "</a><br/>";
					}

					closedir (dir);
				} else {
					_Logger.error("Cannot open dir");
				}

				this->body += "</body></html>";
			}
			else if (SystemFiles::isFile(file_path))
			{
				try
				{
					std::ifstream file(file_path.c_str(), std::ios::binary | std::ios::in);

					if (!file.is_open()) {
						_Logger.error("Failed to open file: " + file_path);
						this->status = "Internal Server Error";
						this->status_code = 500;
						return;
					}

					// Get file size
					file.seekg(0, std::ios::end);
					std::streampos fileSize = file.tellg();
					file.seekg(0, std::ios::beg);

					// Set content-type based on file extension
					String extension = file_path.substr(file_path.find_last_of(".") + 1);
					if (extension == "jpg" || extension == "jpeg") {
						this->setHeader("content-type", "image/jpeg");
					} else if (extension == "png") {
						this->setHeader("content-type", "image/png");
					} else if (extension == "gif") {
						this->setHeader("content-type", "image/gif");
					} else if (extension == "svg") {
						this->setHeader("content-type", "image/svg+xml");
					} else if (extension == "ico") {
						this->setHeader("content-type", "image/x-icon");
					} else if (extension == "css") {
						this->setHeader("content-type", "text/css");
					} else if (extension == "js") {
						this->setHeader("content-type", "application/javascript");
					} else if (extension == "html" || extension == "htm") {
						this->setHeader("content-type", "text/html");
					} else {
						this->setHeader("content-type", "application/octet-stream");
					}

					// Set correct content-length
					this->setHeader("content-length", String::fromInt((size_t)fileSize));

					this->body.reserve(fileSize);
					this->body.assign(std::istreambuf_iterator<char>(file),
								std::istreambuf_iterator<char>());
				}
				catch(const std::exception& e)
				{
					_Logger.error("Exception while reading file: " + String(e.what()));
					this->status = "Internal Server Error";
					this->status_code = 500;
				}
			}
			else
			{
				this->status = "Not Found";
				this->status_code = 404;
			}
		}
};

