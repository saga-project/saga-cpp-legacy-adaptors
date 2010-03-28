# $RCSfile: defrules.mk,v $ $Revision: 1.1 $ $Date: 2007/02/01 10:18:08 $
# $AIST_Release: 5.0.0 $
# $AIST_Copyright:
#  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
#  National Institute of Advanced Industrial Science and Technology
#  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  $

#
# defvar.mk: This file defines rules on Ninf-G build.
#

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o $*.o $*.c

install:: install.bin install.lib install.inc install.doc install.etc \
	install_client.bin install_client.lib install_client.inc \
	install_client.doc install_client.etc \
	install_server.bin install_server.lib install_server.inc \
	install_server.doc install_server.etc

install_client:: install.bin install.lib install.inc install.doc install.etc \
	install_client.bin install_client.lib install_client.inc \
	install_client.doc install_client.etc

install_server:: install.bin install.lib install.inc install.doc install.etc \
	install_server.bin install_server.lib install_server.inc \
	install_server.doc install_server.etc

# Executable install: EXEC_TARGET -> INSTALL_BINDIR
install.bin::	$(EXEC_TARGET)
	@if [ ! -d $(INSTALL_BINDIR) ] ; then \
		(echo "Making directory $(INSTALL_BINDIR)" ; \
		$(MKDIR) $(INSTALL_BINDIR) ; \
		chmod 755 $(INSTALL_BINDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(EXEC_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 755 $$i $(INSTALL_BINDIR); \
				echo "$$i -> $(INSTALL_BINDIR)";; \
		esac; \
	done

install_client.bin::	$(EXEC_CLIENT_TARGET)
	@if [ ! -d $(INSTALL_BINDIR) ] ; then \
		(echo "Making directory $(INSTALL_BINDIR)" ; \
		$(MKDIR) $(INSTALL_BINDIR) ; \
		chmod 755 $(INSTALL_BINDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(EXEC_CLIENT_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 755 $$i $(INSTALL_BINDIR); \
				echo "$$i -> $(INSTALL_BINDIR)";; \
		esac; \
	done

install_server.bin::	$(EXEC_SERVER_TARGET)
	@if [ ! -d $(INSTALL_BINDIR) ] ; then \
		(echo "Making directory $(INSTALL_BINDIR)" ; \
		$(MKDIR) $(INSTALL_BINDIR) ; \
		chmod 755 $(INSTALL_BINDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(EXEC_SERVER_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 755 $$i $(INSTALL_BINDIR); \
				echo "$$i -> $(INSTALL_BINDIR)";; \
		esac; \
	done


# Library install: LIB_TARGET, LIBFILE_TARGET -> INSTALL_LIBDIR
install.lib::	$(LIB_TARGET) $(LIBFILE_TARGET)
	@if [ ! -d $(INSTALL_LIBDIR) ] ; then \
		(echo "Making directory $(INSTALL_LIBDIR)" ; \
		$(MKDIR) $(INSTALL_LIBDIR) ; \
		chmod 755 $(INSTALL_LIBDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(LIB_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_LIBDIR); \
				echo "$$i -> $(INSTALL_LIBDIR)"; \
				$(RANLIB) $(INSTALL_LIBDIR)/$$i;; \
		esac; \
	done
	@for i in / $(LIBFILE_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_LIBDIR); \
				echo "$$i -> $(INSTALL_LIBDIR)"; \
		esac; \
	done


install_client.lib::	$(LIB_CLIENT_TARGET) $(LIBFILE_CLIENT_TARGET)
	@if [ ! -d $(INSTALL_LIBDIR) ] ; then \
		(echo "Making directory $(INSTALL_LIBDIR)" ; \
		$(MKDIR) $(INSTALL_LIBDIR) ; \
		chmod 755 $(INSTALL_LIBDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(LIB_CLIENT_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_LIBDIR); \
				echo "$$i -> $(INSTALL_LIBDIR)"; \
				$(RANLIB) $(INSTALL_LIBDIR)/$$i;; \
		esac; \
	done
	@for i in / $(LIBFILE_CLIENT_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_LIBDIR); \
				echo "$$i -> $(INSTALL_LIBDIR)"; \
		esac; \
	done

install_server.lib::	$(LIB_SERVER_TARGET) $(LIBFILE_SERVER_TARGET)
	@if [ ! -d $(INSTALL_LIBDIR) ] ; then \
		(echo "Making directory $(INSTALL_LIBDIR)" ; \
		$(MKDIR) $(INSTALL_LIBDIR) ; \
		chmod 755 $(INSTALL_LIBDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(LIB_SERVER_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_LIBDIR); \
				echo "$$i -> $(INSTALL_LIBDIR)"; \
				$(RANLIB) $(INSTALL_LIBDIR)/$$i;; \
		esac; \
	done
	@for i in / $(LIBFILE_SERVER_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_LIBDIR); \
				echo "$$i -> $(INSTALL_LIBDIR)"; \
		esac; \
	done

# Include file install: INC_TARGET -> INSTALL_INCDIR
install.inc::	$(INC_TARGET)
	@if [ ! -d $(INSTALL_INCDIR) ] ; then \
		(echo "Making directory $(INSTALL_INCDIR)" ; \
		$(MKDIR) $(INSTALL_INCDIR) ; \
		chmod 755 $(INSTALL_INCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(INC_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_INCDIR); \
				echo "$$i -> $(INSTALL_INCDIR)";; \
		esac; \
	done

install_client.inc::	$(INC_CLIENT_TARGET)
	@if [ ! -d $(INSTALL_INCDIR) ] ; then \
		(echo "Making directory $(INSTALL_INCDIR)" ; \
		$(MKDIR) $(INSTALL_INCDIR) ; \
		chmod 755 $(INSTALL_INCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(INC_CLIENT_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_INCDIR); \
				echo "$$i -> $(INSTALL_INCDIR)";; \
		esac; \
	done

install_server.inc::	$(INC_SERVER_TARGET)
	@if [ ! -d $(INSTALL_INCDIR) ] ; then \
		(echo "Making directory $(INSTALL_INCDIR)" ; \
		$(MKDIR) $(INSTALL_INCDIR) ; \
		chmod 755 $(INSTALL_INCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(INC_SERVER_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_INCDIR); \
				echo "$$i -> $(INSTALL_INCDIR)";; \
		esac; \
	done


# Document/template file install: DOC_TARGET DOCDIR_TARGET -> INSTALL_DOCDIR
install.doc::	$(DOC_TARGET) $(DOCDIR_TARGET)
	@if [ ! -d $(INSTALL_DOCDIR) ] ; then \
		(echo "Making directory $(INSTALL_DOCDIR)" ; \
		$(MKDIR) $(INSTALL_DOCDIR) ; \
		chmod 755 $(INSTALL_DOCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(DOC_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_DOCDIR); \
				echo "$$i -> $(INSTALL_DOCDIR)";; \
		esac; \
	done
	@for i in / $(DOCDIR_TARGET) ; do \
		case $$i in \
			/)  true;; \
			*)  $(TAR) cpf - $$i | \
				( cd $(INSTALL_DOCDIR); $(TAR) xpf -); \
				echo "$$i -> $(INSTALL_DOCDIR)";; \
		esac; \
	done

install_client.doc::	$(DOC_CLIENT_TARGET) $(DOCDIR_CLIENT_TARGET)
	@if [ ! -d $(INSTALL_DOCDIR) ] ; then \
		(echo "Making directory $(INSTALL_DOCDIR)" ; \
		$(MKDIR) $(INSTALL_DOCDIR) ; \
		chmod 755 $(INSTALL_DOCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(DOC_CLIENT_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_DOCDIR); \
				echo "$$i -> $(INSTALL_DOCDIR)";; \
		esac; \
	done
	@for i in / $(DOCDIR_CLIENT_TARGET) ; do \
		case $$i in \
			/)  true;; \
			*)  $(TAR) cpf - $$i | \
				( cd $(INSTALL_DOCDIR); $(TAR) xpf -); \
				echo "$$i -> $(INSTALL_DOCDIR)";; \
		esac; \
	done


install_server.doc::	$(DOC_SERVER_TARGET) $(DOCDIR_SERVER_TARGET)
	@if [ ! -d $(INSTALL_DOCDIR) ] ; then \
		(echo "Making directory $(INSTALL_DOCDIR)" ; \
		$(MKDIR) $(INSTALL_DOCDIR) ; \
		chmod 755 $(INSTALL_DOCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(DOC_SERVER_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_DOCDIR); \
				echo "$$i -> $(INSTALL_DOCDIR)";; \
		esac; \
	done
	@for i in / $(DOCDIR_SERVER_TARGET) ; do \
		case $$i in \
			/)  true;; \
			*)  $(TAR) cpf - $$i | \
				( cd $(INSTALL_DOCDIR); $(TAR) xpf -); \
				echo "$$i -> $(INSTALL_DOCDIR)";; \
		esac; \
	done


# Configuration file install: ETC_TARGET -> INSTALL_ETCDIR
install.etc::	$(ETC_TARGET)
	@if [ ! -d $(INSTALL_ETCDIR) ] ; then \
		(echo "Making directory $(INSTALL_ETCDIR)" ; \
		$(MKDIR) $(INSTALL_ETCDIR) ; \
		chmod 755 $(INSTALL_ETCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(ETC_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_ETCDIR); \
				echo "$$i -> $(INSTALL_ETCDIR)";; \
		esac; \
	done

install_client.etc::	$(ETC_CLIENT_TARGET)
	@if [ ! -d $(INSTALL_ETCDIR) ] ; then \
		(echo "Making directory $(INSTALL_ETCDIR)" ; \
		$(MKDIR) $(INSTALL_ETCDIR) ; \
		chmod 755 $(INSTALL_ETCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(ETC_CLIENT_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_ETCDIR); \
				echo "$$i -> $(INSTALL_ETCDIR)";; \
		esac; \
	done

install_server.etc::	$(ETC_SERVER_TARGET)
	@if [ ! -d $(INSTALL_ETCDIR) ] ; then \
		(echo "Making directory $(INSTALL_ETCDIR)" ; \
		$(MKDIR) $(INSTALL_ETCDIR) ; \
		chmod 755 $(INSTALL_ETCDIR)) ; \
	else \
		true ; \
	fi
	@for i in / $(ETC_SERVER_TARGET) ; do \
		case $$i in \
			/)	true;; \
			*)	$(INSTALL) -m 644 $$i $(INSTALL_ETCDIR); \
				echo "$$i -> $(INSTALL_ETCDIR)";; \
		esac; \
	done

clean::
	$(RM)	$(TARGET) $(OBJS) core *.core *~ $(CLEAN_TARGET)

distclean::	clean
	$(RM)	Makefile config.* .depend* $(DISTCLEAN_TARGET)

