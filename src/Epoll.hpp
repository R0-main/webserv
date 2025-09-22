/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 10:38:34 by rguigneb          #+#    #+#             */
/*   Updated: 2025/09/22 14:25:02 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "utils/Logger.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <map>

typedef struct epoll_event epoll_t;
typedef int fd_t;

class EpollManager
{

	public:
		typedef void (*callback_t)(int, void *);

	private:

		Logger &_logger;

		size_t	_max_events;

		epoll_t *_events;

		std::map<int, std::map<int, callback_t> > fds_callbacks;

		fd_t _ep_fd;

	public:

		typedef enum
		{
			IN = EPOLLIN,
			OUT = EPOLLOUT,
			__BINDING_COUNTS__
		} binding_e;

		EpollManager(Logger &logger, int flags, size_t max_events) : _max_events(max_events), _logger(logger)
		{
			_ep_fd = epoll_create1(flags);
			if (_ep_fd == -1)
			{
				logger.error("fail to epoll_create1()");
				return ;
			}

			_events = new epoll_t[max_events]();
			if (!_events)
			{
				logger.error("fail to allocate epoll_t events");
				return ;
			}
		}

		bool isTracked(int fd) const throw()
		{
			return fds_callbacks.find(fd) != fds_callbacks.end();
		}

		bool isTracked(int fd, int event) throw() // fix const using find
		{
			if (!isTracked(fd))
				return false;
			return fds_callbacks[fd].find(event) != fds_callbacks[fd].end();
		}

		int getTrackedEvents(int fd) throw() // fix const too
		{
			int events = 0;
			if (!isTracked(fd))
				return 0;

			for (int i = IN; i < __BINDING_COUNTS__; i++)
			{
				if (isTracked(fd, i))
					events |= i;
			}

			return events;
		}

		void bindToFd(int fd, uint32_t event, callback_t callback)
		{
			epoll_t tmp;

			if (!isTracked(fd))
			{
				fds_callbacks[fd] = std::map<int, callback_t>();
			}
			else if (isTracked(fd, event))
			{
				_logger.warning("fd already bound for this event");
				return ;
			}

			fds_callbacks[fd][event] = callback;

			tmp.data.fd = fd;
			tmp.events = getTrackedEvents(fd) | event;

			if (epoll_ctl(_ep_fd, EPOLL_CTL_MOD, fd, &tmp) == -1)
			{
				epoll_ctl(_ep_fd, EPOLL_CTL_ADD, fd, &tmp);
			}
			_logger.debug() << "Epoll tracks a new event for fd " << fd << "\n";
		}

		void unbindFd(int fd, int event) throw()
		{
			if (event == -1)
			{
				fds_callbacks[fd].clear();
				epoll_ctl(_ep_fd, EPOLL_CTL_DEL, fd, NULL);
				_logger.debug() << "Epoll untracks a all event for fd " << fd << "\n";
				return;
			}
			if (!isTracked(fd, event))
			{
				_logger.warning("fd is not bound for this event");
				return;
			}
			else
			{
				epoll_t tmp;

				tmp.data.fd = fd;
				tmp.events = getTrackedEvents(fd) & ~event;

				fds_callbacks[fd].erase(event);
				epoll_ctl(_ep_fd, EPOLL_CTL_MOD, fd, &tmp);
				_logger.debug() << "Epoll untracks a event for fd " << fd << "\n";
			}
		}

		void watchForEvents(void *ptr) throw()
		{
			int ready = epoll_wait(_ep_fd, _events, _max_events, 0);

			if (ready == -1)
			{
				_logger.warning("epoll_wait() fails");
				return;
			}

			if (ready == 0)
				return;

			for (size_t i = 0; i < ready; i++)
			{
				int fd = _events[i].data.fd;
				int events = _events[i].events;
				for (int binding = IN; binding < __BINDING_COUNTS__; binding++)
				{
					if (events & binding && isTracked(fd, binding))
					{
						fds_callbacks[fd][binding](fd, ptr);
					}
				}
			}

		}

		~EpollManager()
		{
			close(_ep_fd);
			delete[] _events;
		}
};



		// void trackFd(int fd, uint32_t events) throw()
		// {
		// 	epoll_t tmp;

		// 	tmp.data.fd = fd;
		// 	tmp.events = events;

		// 	epoll_ctl(_ep_fd, EPOLL_CTL_ADD, fd, &tmp);
		// }

		// void modifyFd(int fd, uint32_t events) throw()
		// {
		// 	epoll_t tmp;

		// 	tmp.data.fd = fd;
		// 	tmp.events = events;

		// 	epoll_ctl(_ep_fd, EPOLL_CTL_MOD, fd, &tmp);
		// }
