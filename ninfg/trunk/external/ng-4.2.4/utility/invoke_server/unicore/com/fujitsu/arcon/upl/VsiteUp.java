package com.fujitsu.arcon.upl;

import org.unicore.Vsite;

/**
 * A request sent by a Vsite (NJS) to a Gateway that the Gateway
 * starts to provide access to the Vsite.
 * <p>
 * If the VsiteUp request is accepted, then the Gateway will return
 * the Vsite in its {@link org.unicore.upl.ListVsitesReply} and will
 * route all {@link org.unicore.upl.ServerRequest} for the Vsite to the
 * Vsite address sent with this request.
 * <p>
 * The Gateway will do this for the lifetime contained in this instance (or until
 * the Gateway is restarted). Once the lifetime has expired the Vsite will be removed from
 * the list.
 * <p>
 * The information in this request replaces any information that the Gateway may
 * hold about the Vsite from any previous VsiteUp requests or any initialisation.
 * <p>
 * A Gateway does not have to accept a VsiteUp request and depending on 
 * the implementation can use a number of criteria to decide whether or not
 * to accept the request. For example, Gateways can be initialised to accept
 * VsiteUp requests only from certain Vsites.
 *
 * @author S. van den Berghe, Fujitsu Laboratories of Europe
 *
 * @since AJO 4.0
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 *
 **/
public final class VsiteUp extends org.unicore.upl.Request {

    static final long serialVersionUID = 400;

    public VsiteUp() {
    }

    public VsiteUp(Vsite vsite) {
        this.vsite = vsite;
    }

    private Vsite vsite;

    /**
     * Return the Vsite details.
     * <p>
     * These details will be sent by the Gateway in its {@link org.unicore.upl.ListVsitesReply}
     * with the Vsite's address field replaced by the address of the Gateway.
     * <p>
     * The Gateway will use the address in this instance to contact the NJS.
     *
     * @return The Vsite details
     *
     **/
    public Vsite getVsite() {return vsite;}

    /**
     * Set the Vsite details
     *
     * @param vsite The vsite details
     *
     **/
    public void setVsite(Vsite vsite) {this.vsite = vsite;}

    private byte[] further_information;

    /**
     * Get the extra information, this is placed into the ListVsitesReply
     * further information array {@link org.unicore.upl.ListVsitesReply#getFurtherInformation}
     *
     * @return The extra information
     *
     **/
    public byte[] getFurtherInformation() {return further_information;}

    /**
     * Set the extra information.
     *
     * @param further_information The extra information
     *
     **/
    public void setFurtherInformation(byte[] further_information) {this.further_information = further_information;}

	private long lifetime;

	/**
	 * Return how long the Gateway will hold this instance (milliseconds).
	 *
	 **/
	public long getLifetime() {return lifetime;}

	public void setLifetime(long lifetime) {this.lifetime = lifetime;}

}
//
//                   Copyright (c) Fujitsu Ltd 2000 - 2004
//
//                Use and distribution is subject a License.
// A copy was supplied with the distribution (see documentation or the jar file).
//
// This product includes software developed by Fujitsu Limited (http://www.fujitsu.com).
