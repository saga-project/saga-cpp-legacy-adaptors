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

SET search_path = public, pg_catalog;

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
-- Name: attributes_key_key; Type: CONSTRAINT; Schema: public; Owner: SAGA; Tablespace: 
--

ALTER TABLE ONLY attributes
    ADD CONSTRAINT attributes_key_key UNIQUE (key);


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

