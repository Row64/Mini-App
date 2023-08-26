#ifndef CH_APPMESSAGE_H
#define CH_APPMESSAGE_H

#pragma once

namespace AppCore {

    // ************************************************************ //
    // AppMessage Struct                                            //
    // ************************************************************ //
    struct AppMessage {
        std::string Filter;
        std::string Type;
        std::vector<std::string> Message;
    };

} // end namespace AppCore

#endif /* CH_APPMESSAGE_H */
