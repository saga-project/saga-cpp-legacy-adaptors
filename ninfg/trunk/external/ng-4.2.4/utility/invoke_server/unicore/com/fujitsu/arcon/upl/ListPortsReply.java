package com.fujitsu.arcon.upl;

import org.unicore.Vsite;

/**
 * A ListPortsReply contains a list of the Ports at a Usite.
 *
 * @author S. van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 * @since AJO 4.0
 *
 **/

public final class ListPortsReply extends org.unicore.upl.Reply {

    public ListPortsReply() {
    }

    public ListPortsReply(Vsite[] vsites) {
        this.vsites = vsites;
    }

    private Vsite[] vsites;

    public Vsite[] getList() {return vsites;}

    public void setList(Vsite[] vsites) {this.vsites = vsites;}

    private byte[][] further_information;

    public byte[][] getFurtherInformation() {return further_information;}

    public void setFurtherInformation(byte[][] further_information) {
        this.further_information = further_information;
    }

}
//
//                   Copyright (c) Fujitsu Ltd 2000 - 2004
//
//                Use and distribution is subject a License.
// A copy was supplied with the distribution (see documentation or the jar file).
//
// This product includes software developed by Fujitsu Limited (http://www.fujitsu.com).
