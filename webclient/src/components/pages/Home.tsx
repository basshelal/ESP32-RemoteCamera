import {Element, FC} from "../../Utils"

export interface HomeProps {}

export const Home: FC<HomeProps> = (): Element => {

    return (<main class="container">
        <h1>Home</h1>
    </main>)
}