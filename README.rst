===========
vmod_geoip2
===========

----------------------------
Varnish GeoIP2 Lookup Module
----------------------------

:Author: Karl von Randow, Hauke Lampe
:Date: 2020-01-09
:Version: 1.0.3
:Manual section: 3

SYNOPSIS
========

import geoip2;

DESCRIPTION
===========

This Varnish module exports functions to look up GeoIP2 country codes.
Requires MaxMind's libmaxminddb library (on Debian install libmaxminddb)

Forked from https://github.com/varnish/libvmod-geoip (which uses the legacy GeoIP library)
and using code from https://github.com/nytm/varnish-mmdb-vmod


FUNCTIONS
=========

init
----

Prototype
        ::

                init(STRING mmdb_path)
Return value
        INT
Description
        Load an mmdb file.
Example
        ::

                geoip2.init("/var/lib/mddb/GeoLite2-City.mmdb");

country_code
------------

Prototype
        ::

                country_code(STRING S)
Return value
	STRING
Description
	Returns two-letter country code string
Example
        ::

                set req.http.X-Country-Code = geoip2.country_code("127.0.0.1");

country_name
------------

Prototype
        ::

                country_name(STRING S)
Return value
	STRING
Description
	Returns country name string
Example
        ::

                set req.http.X-Country-Name = geoip2.country_name("127.0.0.1");

region_code
------------------------------

Prototype
        ::

                region_code(STRING S)
Return value
        STRING
Description
        Returns region code string
Example
        ::

                set req.http.X-Region-Code = geoip2.region_code("127.0.0.1");

region_name
------------------------------

Prototype
        ::

                region_name(STRING S)
Return value
	STRING
Description
	Returns region name string
Example
        ::

                set req.http.X-Region-Name = geoip2.region_name("127.0.0.1");

city_name
------------------------------

Prototype
        ::

                city_name(STRING S)
Return value
        STRING
Description
        Returns city name string
Example
        ::

                set req.http.X-City-Name = geoip2.city_name("127.0.0.1");


INSTALLATION
============

The source tree is based on autotools to configure the building, and
does also have the necessary bits in place to do functional unit tests
using the varnishtest tool.

It is a prerequisite that you have the varnish-dev package installed as well as varnish::

 apt-get install varnish-dev

Install the MaxMindDB library headers::

 apt-get install libmaxminddb-dev

To check out the current development source::

 git clone git://github.com/karlvr/libvmod-geoip2.git
 cd libvmod-geoip2; ./autogen.sh

Usage::

 ./configure

Make targets:

* make - builds the vmod
* sudo make install - installs your vmod
* make check - runs the unit tests in ``src/tests/*.vtc``

Database:

The tests rely on an mmdb being available at /var/lib/mmdb/GeoLite2-City.mmdb
