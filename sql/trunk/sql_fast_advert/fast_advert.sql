--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- Name: fast_advert; Type: DATABASE; Schema: -; Owner: postgres
--

CREATE DATABASE fast_advert WITH TEMPLATE = template0 ENCODING = 'UTF8';


ALTER DATABASE fast_advert OWNER TO postgres;

\connect fast_advert

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- Name: plpgsql; Type: PROCEDURAL LANGUAGE; Schema: -; Owner: postgres
--

CREATE OR REPLACE PROCEDURAL LANGUAGE plpgsql;


ALTER PROCEDURAL LANGUAGE plpgsql OWNER TO postgres;

SET search_path = public, pg_catalog;

--
-- Name: insert_node(character varying, boolean, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: SAGA
--

CREATE FUNCTION insert_node(n character varying, d boolean, l integer, r integer, h integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
declare
node record;
begin
update nodes set rgt = rgt + 2 where rgt > l;
update nodes set lft = lft  + 2 where lft > l;
insert into nodes (name, dir, lft, rgt, hash) values (n, d, l + 1, l + 2, h);
select * into node from nodes where hash = h;
return node;
end;
$$;


ALTER FUNCTION public.insert_node(n character varying, d boolean, l integer, r integer, h integer) OWNER TO "SAGA";

--
-- Name: remove_node(integer, integer); Type: FUNCTION; Schema: public; Owner: SAGA
--

CREATE FUNCTION remove_node(l integer, r integer) RETURNS void
    LANGUAGE plpgsql
    AS $$
declare
node record;
begin
for node in delete from nodes where nodes.lft between l and r returning nodes.id loop
delete from attributes where node_id = node.id;
delete from data where node_id = node.id;
end loop;
update nodes set rgt = rgt - (r - l + 1) where rgt > r;
update nodes set lft = lft - (r - l + 1) where lft > r;
end;
$$;


ALTER FUNCTION public.remove_node(l integer, r integer) OWNER TO "SAGA";

--
-- Name: return_int(integer); Type: FUNCTION; Schema: public; Owner: SAGA
--

CREATE FUNCTION return_int(input integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$
declare
begin
return input;
end;
$$;


ALTER FUNCTION public.return_int(input integer) OWNER TO "SAGA";

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: attributes; Type: TABLE; Schema: public; Owner: SAGA; Tablespace: 
--

CREATE TABLE attributes (
    node_id integer NOT NULL,
    key character varying(256) NOT NULL,
    value character varying(256) NOT NULL,
    is_vector boolean NOT NULL
);


ALTER TABLE public.attributes OWNER TO "SAGA";

--
-- Name: data; Type: TABLE; Schema: public; Owner: SAGA; Tablespace: 
--

CREATE TABLE data (
    node_id integer NOT NULL,
    data character varying NOT NULL
);


ALTER TABLE public.data OWNER TO "SAGA";

--
-- Name: nodes; Type: TABLE; Schema: public; Owner: SAGA; Tablespace: 
--

CREATE TABLE nodes (
    id integer NOT NULL,
    name character varying(256) NOT NULL,
    dir boolean NOT NULL,
    lft integer NOT NULL,
    rgt integer NOT NULL,
    hash integer NOT NULL
);


ALTER TABLE public.nodes OWNER TO "SAGA";

--
-- Name: nodes_id_seq; Type: SEQUENCE; Schema: public; Owner: SAGA
--

CREATE SEQUENCE nodes_id_seq
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.nodes_id_seq OWNER TO "SAGA";

--
-- Name: nodes_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: SAGA
--

ALTER SEQUENCE nodes_id_seq OWNED BY nodes.id;


--
-- Name: nodes_id_seq; Type: SEQUENCE SET; Schema: public; Owner: SAGA
--

SELECT pg_catalog.setval('nodes_id_seq', 55, true);


--
-- Name: version; Type: TABLE; Schema: public; Owner: SAGA; Tablespace: 
--

CREATE TABLE version (
    layout_version character varying(8)
);


ALTER TABLE public.version OWNER TO "SAGA";

--
-- Name: id; Type: DEFAULT; Schema: public; Owner: SAGA
--

ALTER TABLE nodes ALTER COLUMN id SET DEFAULT nextval('nodes_id_seq'::regclass);


--
-- Data for Name: attributes; Type: TABLE DATA; Schema: public; Owner: SAGA
--



--
-- Data for Name: data; Type: TABLE DATA; Schema: public; Owner: SAGA
--



--
-- Data for Name: nodes; Type: TABLE DATA; Schema: public; Owner: SAGA
--

INSERT INTO nodes VALUES (1, 'root', true, 1, 2, -86386329);


--
-- Data for Name: version; Type: TABLE DATA; Schema: public; Owner: SAGA
--

INSERT INTO version VALUES ('1.0');


--
-- Name: attributes_node_id_key; Type: CONSTRAINT; Schema: public; Owner: SAGA; Tablespace: 
--

ALTER TABLE ONLY attributes
    ADD CONSTRAINT attributes_node_id_key UNIQUE (node_id, key);


--
-- Name: nodes_pkey; Type: CONSTRAINT; Schema: public; Owner: SAGA; Tablespace: 
--

ALTER TABLE ONLY nodes
    ADD CONSTRAINT nodes_pkey PRIMARY KEY (id);


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

