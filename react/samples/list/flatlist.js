import React from "react";
import ReactDOM from "react-dom";
import { Window, View, FlatList, Text, StyleSheet } from "../../lib/core";

const DATA = [
  {
    id: "bd7acbea-c1b1-46c2-aed5-3ad53abb28ba",
    title: "First Item"
  },
  {
    id: "3ac68afc-c605-48d3-a4f8-fbd91aa97f63",
    title: "Second Item"
  },
  {
    id: "58694a0f-3da1-471f-bd96-145571e29d72",
    title: "Third Item"
  }
];

function Item({ title }) {
  return (
    <View style={styles.item} onPress={(evt)=>{console.log(title)}}>
      <Text style={styles.title}>{title}</Text>
    </View>
  );
}

function App() {
  return (
    <Window>
      <View>
        <FlatList
          data={DATA}
          renderItem={({ item }) => <Item title={item.title} />}
          keyExtractor={(item, index) => item + index}
        />
      </View>
    </Window>
  );
}


const styles = StyleSheet.create({
  item: {
    backgroundColor: '#f9c2ff',
    padding: 20,
    marginVertical: 8,
    marginHorizontal: 16
  },
  title: {
    fontSize: 32
  },
});

ReactDOM.render(<App />, document.getElementById("root"));
