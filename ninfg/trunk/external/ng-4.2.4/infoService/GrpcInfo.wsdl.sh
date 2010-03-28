#! /bin/sh
# $RCSfile: GrpcInfo.wsdl.sh,v $ $Revision: 1.1 $ $Date: 2008/08/19 10:53:28 $
# $AIST_Release: 4.2.4 $
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

. ./config.sh

XMLNS_WSRP="http://docs.oasis-open.org/wsrf/2004/06/wsrf-WS-ResourceProperties-1.2-draft-01.xsd"
XMLNS_WSRPW="http://docs.oasis-open.org/wsrf/2004/06/wsrf-WS-ResourceProperties-1.2-draft-01.wsdl"
XMLNS_WSNT="http://docs.oasis-open.org/wsn/2004/06/wsn-WS-BaseNotification-1.2-draft-01.xsd"
XMLNS_WSNTW="http://docs.oasis-open.org/wsn/2004/06/wsn-WS-BaseNotification-1.2-draft-01.wsdl"
WSRPW_LOCATION="../wsrf/properties/WS-ResourceProperties.wsdl"
WSNTW_LOCATION="../wsrf/notification/WS-BaseN.wsdl"
if test "x$GT42" = "xyes"; then
    XMLNS_WSRP="http://docs.oasis-open.org/wsrf/rp-2"
    XMLNS_WSRPW="http://docs.oasis-open.org/wsrf/rpw-2"
    XMLNS_WSNT="http://docs.oasis-open.org/wsn/b-2"
    XMLNS_WSNTW="http://docs.oasis-open.org/wsn/bw-2"
    WSRPW_LOCATION="../wsrf/properties/rpw-2.wsdl"
    WSNTW_LOCATION="../wsrf/notification/bw-2.wsdl"
fi

cat << EOS
<?xml version="1.0" encoding="UTF-8"?>
<definitions
		name="GrpcInfoService"
		targetNamespace="http://ninf.apgrid.org/ng4/grpcinfo"
		xmlns="http://schemas.xmlsoap.org/wsdl/"
		xmlns:ng4gi="http://ninf.apgrid.org/ng4/grpcinfo"
		xmlns:ng4gitypes="http://ninf.apgrid.org/ng4/grpcinfo/types"
		xmlns:wsa="http://schemas.xmlsoap.org/ws/2004/03/addressing"
		xmlns:wsrp="${XMLNS_WSRP}"
		xmlns:wsrpw="${XMLNS_WSRPW}"
		xmlns:wsnt="${XMLNS_WSNT}"
		xmlns:wsntw="${XMLNS_WSNTW}"
		xmlns:wsdlpp="http://www.globus.org/namespaces/2004/10/WSDLPreprocessor">
	<import
			namespace="${XMLNS_WSRPW}"
			location="${WSRPW_LOCATION}"/>
	<import
			namespace="${XMLNS_WSNTW}"
			location="${WSNTW_LOCATION}"/>
	<types>
		<xsd:schema targetNamespace="http://ninf.apgrid.org/ng4/grpcinfo/types"
				xmlns:xsd="http://www.w3.org/2001/XMLSchema">
			<!-- Requests and responses -->
			<xsd:complexType name="RescanRequestType"/>
			<xsd:element name="rescan" type="ng4gitypes:RescanRequestType"/>
			<xsd:complexType name="RescanResponseType"/>
			<xsd:element name="rescanResponse" type="ng4gitypes:RescanResponseType"/>
			<!-- Resource properties -->
			<xsd:complexType name="GrpcInfoSetType">
				<xsd:sequence>
					<xsd:any namespace="##any" minOccurs="1" maxOccurs="1"/>
				</xsd:sequence>
			</xsd:complexType>
			<xsd:element name="grpcInfoSet" type="ng4gitypes:GrpcInfoSetType"/>
			<xsd:complexType name="GrpcInfoRPType">
				<xsd:sequence>
					<xsd:element ref="ng4gitypes:grpcInfoSet" minOccurs="1" maxOccurs="1"/>
				</xsd:sequence>
			</xsd:complexType>
			<xsd:element name="grpcInfoRP" type="ng4gitypes:GrpcInfoRPType"/>
		</xsd:schema>
	</types>
	<message name="RescanRequestMessage">
		<part name="args" element="ng4gitypes:rescan"/>
	</message>
	<message name="RescanResponseMessage">
		<part name="result" element="ng4gitypes:rescanResponse"/>
	</message>
	<portType
			name="GrpcInfoPortType"
			wsdlpp:extends="wsrpw:QueryResourceProperties wsntw:NotificationProducer"
			wsrp:ResourceProperties="ng4gitypes:grpcInfoRP">
		<operation name="rescan" parameterOrder="args">
			<input
					name="rescan"
					message="ng4gi:RescanRequestMessage"
					wsa:Action="http://ninf.apgrid.org/ng4/grpcinfo/GrpcInfoPortType/rescan"/>
			<output
					name="rescanResponse"
					message="ng4gi:RescanResponseMessage"
					wsa:Action="http://ninf.apgrid.org/ng4/grpcinfo/GrpcInfoPortType/rescanResponse"/>
		</operation>
	</portType>
</definitions>
EOS
