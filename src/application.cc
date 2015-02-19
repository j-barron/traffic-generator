/*
 * Traffic generator
 *
 *   Addy Bombeke          <addy.bombeke@ugent.be>
 *   Douwe De Bock         <douwe.debock@ugent.be>
 *   Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <iostream>

#define RINA_PREFIX "traffic-generator"

#include <librina/logs.h>
#include <librina/librina.h>

#include "application.h"

using namespace std;
using namespace rina;

void Application::applicationRegister()
{
        ApplicationRegistrationInformation ari;
        RegisterApplicationResponseEvent * resp;
        unsigned int seqnum;
        IPCEvent * event;

        ari.ipcProcessId = 0;  // This is an application, not an IPC process
        ari.appName = ApplicationProcessNamingInformation(appName,
                        appInstance);

        if (difName == string()) {
                ari.applicationRegistrationType =
                        ApplicationRegistrationType::APPLICATION_REGISTRATION_ANY_DIF;
        } else {
                ari.applicationRegistrationType =
                        ApplicationRegistrationType::APPLICATION_REGISTRATION_SINGLE_DIF;
                ari.difName = ApplicationProcessNamingInformation(difName, string());
        }

        // Request the registration
        seqnum = ipcManager->requestApplicationRegistration(ari);

        // Wait for the response to come
        for (;;) {
                event = ipcEventProducer->eventWait();
                if (event && event->eventType ==
                                REGISTER_APPLICATION_RESPONSE_EVENT &&
                                event->sequenceNumber == seqnum) {
                        break;
                }
        }

        resp = dynamic_cast<RegisterApplicationResponseEvent*>(event);

        // Update librina state
        if (resp->result == 0) {
                ipcManager->commitPendingRegistration(seqnum, resp->DIFName);
        } else {
                ipcManager->withdrawPendingRegistration(seqnum);
                throw ApplicationRegistrationException("Failed to register application");
        }
}

// FIXME: Are we sure ?
const unsigned int Application::maxBufferSize = 1 << 16;
