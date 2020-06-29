import React from 'react';
import ReactDOM from 'react-dom';
import { v4 as uuid } from 'uuid';

import qt from '../../lib/engine';
import {
    Window,
    View,
    Image,
    Text,
    TextInput,
    Button,
    Switch,
    ScrollView,
    SplitterView,
    FlatList,
    SectionList,
    StatusBar,
    MenuBar,
    Menu,
    MenuItem,
    StyleSheet
} from '../../lib/core';

import { useTodos, StoreProvider as TodosProvider } from './context';

const TodoItem = ({ item, toggleTodo, deleteTodo, index }) => {
    let Wrap = React.Fragment;
    if (item.checked) {
        Wrap = 's';
    }

    return (
        <View id={`todo-item-${index}`} retained style={{ ...styles.item, flexDirection: 'row' }}>
            <Switch
                style={styles.check}
                text="check"
                value={item.checked}
                // style={{ background: "red", border: "none" }}
                onClick={evt => {
                    toggleTodo(item);
                }}
            />
            <Text style={{ flex: 2 }}>
                <Wrap>{index} {item.text}</Wrap>
            </Text>
            <Button
                text="delete"
                onClick={evt => {
                    deleteTodo(item);
                }}
            />
        </View>
    );
};

const App = () => {
    const todos = useTodos();
    const state = todos.state;

    const addTodo = evt => {
        if (state.newTodo === '') {
            return;
        }
        todos.dispatch(
            todos.setState({
                ...state,
                newTodo: '',
                todos: [
                    ...state.todos,
                    {
                        id: uuid(),
                        text: state.newTodo,
                        checked: false
                    }
                ]
            })
        );
    };

    const toggleTodo = todo => {
        let idx = state.todos.findIndex(t => {
            return t.id === todo.id;
        });

        if (idx !== -1) {
            let updateTodos = [...state.todos];
            updateTodos[idx].checked = !updateTodos[idx].checked;
            todos.dispatch(
                todos.setState({
                    ...state,
                    todos: updateTodos
                })
            );
        }
    };

    const deleteTodo = todo => {
        let idx = state.todos.findIndex(t => {
            return t.id === todo.id;
        });

        if (idx !== -1) {
            let updateTodos = [...state.todos];
            updateTodos.splice(idx, 1);
            todos.dispatch(
                todos.setState({
                    ...state,
                    todos: updateTodos
                })
            );
        }
    };

    const setNewTodo = evt => {
        todos.dispatch(
            todos.setState({
                ...state,
                newTodo: evt.target.value
            })
        );
    };

    const todosRendered = state.todos.map((todo, idx) => {
        let Wrap = React.Fragment;
        if (todo.checked) {
            Wrap = 's';
        }
        return (
            <View key={`todo-${idx}`} style={{ flexDirection: 'row' }}>
                <Button
                    text="check"
                    // style={{ background: "red", border: "none" }}
                    onClick={evt => {
                        toggleTodo(todo);
                    }}
                />
                <Text style={{ flex: 2 }}>
                    <Wrap>{todo.text}</Wrap>
                </Text>
                <Button
                    text="delete"
                    onClick={evt => {
                        deleteTodo(todo);
                    }}
                />
            </View>
        );
    });

    const onExit = evt => {
        console.log('bye!');
    };

    return (
        <Window
            id="mainWindow"
            style={{ ...styles.container, flexDirection: 'column' }}
        >
            <StatusBar>
                <Text permanent>{state.newTodo}</Text>
                <Text>123</Text>
            </StatusBar>
            <MenuBar>
                <Menu text="&File">
                    <MenuItem text="&New" />
                    <MenuItem text="&Open" />
                    <MenuItem text="&Save" />
                    <MenuItem text="E&xit" onClick={onExit} />
                </Menu>
                <Menu text="&View"></Menu>
                <Menu text="&Help"></Menu>
            </MenuBar>
            <View style={{ flexDirection: 'row', flex: 0 }}>
            <Text>
                <h2>Todos App</h2>
            </Text>
            <Image
                source="image-placeholder.jpg"
                style={{ width: 200, height: 200 }}
            />
            </View>
            <View style={{ flexDirection: 'row', flex: 0 }}>
                <TextInput
                    text={state.newTodo}
                    onSubmitEditing={addTodo}
                    onChangeText={setNewTodo}
                ></TextInput>
                <Button text="Add Todo" onClick={addTodo}></Button>
            </View>
            <SplitterView style={{ flex: 2 }}>
                <ScrollView>
                    <View
                        style={{
                            flexDirection: 'column',
                            alignItems: 'flex-start'
                        }}
                    >
                        {todosRendered}
                    </View>
                </ScrollView>
                <FlatList
                    data={state.todos}
                    renderItem={props => <TodoItem {...props} />}
                    style={{ flex: 1 }}
                    extraData={{ toggleTodo, deleteTodo }}
                ></FlatList>
            </SplitterView>
        </Window>
    );
};

const styles = StyleSheet.create({
    container: {
        flex: 1,
        // marginTop: Constants.statusBarHeight,
        marginHorizontal: 16,
        backgroundColor: '#fff',
        border: 'none'
    },
    item: {
        backgroundColor: '#f9c2ff',
        padding: 20,
        marginVertical: 8,
        marginHorizontal: 8
    },
    title: {
        fontSize: 32
    },
    check: {
        border: 'none'
    }
});

ReactDOM.render(
    <TodosProvider>
        <App />
    </TodosProvider>,
    document.getElementById('root')
);
