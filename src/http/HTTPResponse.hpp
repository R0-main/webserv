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

#define STYLE "<style> /* Modern directory listing — drop-in, no HTML changes needed */ :root{ --bg:#0b0f14; --panel:#0f1520; --muted:#8da2b8; --fg:#e6edf3; --accent:#7c9cff; --ring:color-mix(in oklab,var(--accent),white 25%); --border:color-mix(in oklab,var(--panel),white 10%); --shadow:0 1px 1px rgba(0,0,0,.04),0 6px 16px rgba(0,0,0,.18); } @media (prefers-color-scheme: light){ :root{ --bg:#f7f9fc; --panel:#ffffff; --muted:#5b6b7b; --fg:#0b1220; --accent:#3557ff; --ring:color-mix(in oklab,var(--accent),black 10%); --border:color-mix(in oklab,var(--panel),black 12%); --shadow:0 1px 1px rgba(0,0,0,.04),0 6px 16px rgba(0,0,0,.08); } } *{box-sizing:border-box} html,body{height:100%} body{ margin:0; padding:clamp(16px,3vw,32px); background: radial-gradient(1200px 600px at 100% -10%,color-mix(in oklab,var(--accent),transparent 92%),transparent), radial-gradient(1000px 600px at -10% 120%,color-mix(in oklab,var(--accent),transparent 92%),transparent), var(--bg); color:var(--fg); font:500 16px/1.4 ui-sans-serif,system-ui,-apple-system,Segoe UI,Roboto,Inter,\"Helvetica Neue\",Arial,\"Noto Sans\",\"Apple Color Emoji\",\"Segoe UI Emoji\"; color-scheme:light dark; display:grid; align-content:start; gap:14px; } br{display:none} /* render links as cards instead */ a{ display:flex; align-items:center; gap:12px; padding:14px 16px; border-radius:16px; background:var(--panel); border:1px solid var(--border); box-shadow:var(--shadow); text-decoration:none; color:inherit; min-width:0; /* enables ellipsis */ transition:transform .12s ease,box-shadow .2s ease,border-color .2s ease,background .2s ease; will-change:transform; white-space:nowrap;overflow:hidden;text-overflow:ellipsis; } a:hover{ transform:translateY(-2px); box-shadow:0 10px 24px rgba(0,0,0,.2); border-color:var(--ring); } a:focus-visible{ outline:none; box-shadow:0 0 0 3px color-mix(in oklab,var(--ring),transparent 60%),var(--shadow); border-color:var(--ring); } a::first-letter{ /* make the emoji icon pop */ font-size:1.35em; line-height:0; } a::after{ /* subtle caret */ content:\"↗\"; margin-left:auto; opacity:.25; transition:opacity .2s ease,transform .12s ease; } a:hover::after{opacity:.7;transform:translateX(2px)} a:active{transform:translateY(0)} ::selection{background:color-mix(in oklab,var(--accent),transparent 70%)} /* Responsive column layout when there are many links */ @supports (width: min(100%, 640px)){ body{grid-template-columns:repeat(auto-fill,minmax(min(100%,420px),1fr))} } /* Scrollbar polish (WebKit) */ ::-webkit-scrollbar{width:10px;height:10px} ::-webkit-scrollbar-thumb{ background:color-mix(in oklab,var(--fg),transparent 85%); border-radius:999px;border:2px solid transparent;background-clip:content-box } ::-webkit-scrollbar-track{background:transparent} </style>"
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

