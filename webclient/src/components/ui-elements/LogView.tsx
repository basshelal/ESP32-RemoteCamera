import {FC, JSXElement} from "../../Utils"

const LogView: FC = (): JSXElement => {
    // TODO: Use websocket to get instant notifications from server about new entries,
    //  on first load just get all data from server (as much as it can hold), new entries will be added
    //  to webpage likely more than what the server can hold? or do we keep parity with server?
    //  depends on our implementation of websocket communication
    return (<>
    </>)
}