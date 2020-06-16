import React from "react";
import ReactDOM from "react-dom";
import { Window, View, SectionList, Text, StyleSheet } from "../../lib/core";

const DATA = [
  {
    title: "Main dishes",
    data: ["Pizza", "Burger", "Risotto"]
  },
  {
    title: "Sides",
    data: ["French Fries", "Onion Rings", "Fried Shrimps"]
  },
  {
    title: "Drinks",
    data: ["Water", "Coke", "Beer"]
  },
  {
    title: "Desserts",
    data: ["Cheese Cake", "Ice Cream"]
  }
];

function Item({ title }) {
  return (
    <View
      style={styles.item}
      onPress={evt => {
        console.log(title);
      }}
    >
      <Text style={styles.title}>{title}</Text>
    </View>
  );
}

function App() {
  return (
    <Window id='mainWindow' style={styles.container} qss={qss}>
      <View style={styles.container}>
        <SectionList
          style={{margin:0, padding:0}}
          data={DATA}
          renderItem={({ item }) => <Item title={item} />}
          renderSectionHeader={({ section: { title } }) => (
            <Text style={styles.header}>{title}</Text>
          )}
          keyExtractor={(item, index) => item + index}
        />
      </View>
    </Window>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#fff",
    border: "none"
  },
  item: {
    backgroundColor: "#f9c2ff",
    padding: 20,
    marginVertical: 8,
    marginHorizontal: 8
  },
  header: {
    fontSize: 32,
    padding: 20,
    backgroundColor: "#fff"
  },
  title: {
    margin: 0,
    padding: 0,
    fontSize: 24
  }
});

const qss = StyleSheet.create({
  'QScrollBar:vertical': {
    backgroundColor: 'yellow'
  }
});

ReactDOM.render(<App />, document.getElementById("root"));
