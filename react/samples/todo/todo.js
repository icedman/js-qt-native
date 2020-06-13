import React from "react";
import ReactDOM from "react-dom";
import { v4 as uuid } from 'uuid';

import qt from '../../lib/qt';
import {
    MainWindow,
    View,
    Text,
    TextInput,
    Button
} from '../../lib/core';

const App = () => {
  const [state, setState] = React.useState({
    todos: []
  });

  const addTodo = (evt) => {
    setState({
      ...state,
      newTodo: '',
      todos: [ ...state.todos, {
        id: uuid(),
        text: state.newTodo,
        checked: false
      }]
    })
  };

  const deleteTodo = (todo) => {
    let idx = state.todos.findIndex((t) => {
      return t.id === todo.id;
    });

    if (idx !== -1) {
      let updateTodos = [ ...state.todos ];
      updateTodos.splice(idx, 1);
      setState({
        ...state,
        todos: updateTodos
      })
    }
  }

  const setNewTodo = (evt) => {
    setState({
        ...state,
        newTodo: evt.target.value
    })
  }

  const todosRendered = state.todos.map((todo,idx) => {
    return <View key={`todo-${idx}`}>
        <Text text={todo.text}/>
        <Button text='delete' onClick={(evt) =>{ deleteTodo(todo) }}/>
      </View>
  })

    return <div>
        <MainWindow id='mainWindow' style={{flexDirection:'column'}}>
            <Text>
                <h2>Todos App</h2>
            </Text>
            <TextInput text={state.newTodo} onSubmit={addTodo} onChange={setNewTodo}></TextInput>
            <Button text='Add Todo' onClick={addTodo}></Button>
            <View style={{flexDirection:'column'}}>
            {todosRendered}
            </View>
        </MainWindow>
        </div>
}

ReactDOM.render(<App />, document.getElementById("root"));