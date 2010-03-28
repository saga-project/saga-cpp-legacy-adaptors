
OPTIONS


    rsl_extensions <extension>

        This option specify RSL extension.


    gsiftp_port <port> (default: 2811)

        This option specify port # of GSIFTP on client host.

      EXAMPLE

        <SERVER>
          invoke_server_option "gsiftp_port 12811"
        </SERVER>


    subject <subject string>

        This option specify subject string of server.

     EXAMPLE

        <SERVER>
          invoke_server_option "subject /C=JP/O=AIST/OU=GRID/CN=YAGYU Jubei"
        </SERVER>


    staging_destination_subject <subject string>

        This option specify subject string of staging destination.

     EXAMPLE

        <SERVER>
          invoke_server_option "staging_destination_subject /C=JP/O=AIST/OU=GRID/CN=MIYAMOTO Musashi"
        </SERVER>


    staging_source_subject <subject string>

        This option specify subject string of staging source.

     EXAMPLE

        <SERVER>
          invoke_server_option "staging_source_subject /C=JP/O=AIST/OU=GRID/CN=TSUKAHARA Bokuden"
        </SERVER>


    deletion_subject <subject string>

        This option specify subject string for deletion operation.

     EXAMPLE

        <SERVER>
          invoke_server_option "deletion_subject /C=JP/O=AIST/OU=GRID/CN=CHIBA Shusaku"
        </SERVER>


    protocol [ http | https | other... ]

        This option specify communication protocol for WS container.

     EXAMPLE

        <SERVER>
          invoke_server_option "protocol http"
        </SERVER>

    delegate_full_proxy [ true / false ] (default false)

        This option specify to delegate full proxy or limited proxy.
        The default is false and limited proxy are used.
        If the user want to use GridRPC API on the servers,
        full proxy delegation must be used.

ENVIRONMENT VARIABLES

  TMPDIR, TEMP, TMP
    The temporary file directory to place the Invoke Server temporary
   files. (Delegated credentials, GRAM RSLs, ...)

