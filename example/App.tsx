/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * @format
 * @flow strict-local
 */

import React from 'react';
import type {Node} from 'react';
import {useState} from 'react';

import {
  SafeAreaView,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  useColorScheme,
  View,
} from 'react-native';

import {
  Colors,
  DebugInstructions,
  Header,
  LearnMoreLinks,
  ReloadInstructions,
} from 'react-native/Libraries/NewAppScreen';

import {HyperlinkButton, Border, TextBlock, StackPanel, Button, ComboBox, ComboBoxItem,
NativeXamlControl,
MenuFlyoutItem,
TextBox,
BlankUserControl,
} from 'react-native-xaml'; // Would be from 'react-native-xaml' outside of this repo;

const Section = ({children, title}): Node => {
  const isDarkMode = useColorScheme() === 'dark';
  return (
    <View style={styles.sectionContainer}>
      <Text
        style={[
          styles.sectionTitle,
          {
            color: isDarkMode ? Colors.white : Colors.black,
          },
        ]}>
        {title}
      </Text>
      <Text
        style={[
          styles.sectionDescription,
          {
            color: isDarkMode ? Colors.light : Colors.dark,
          },
        ]}>
        {children}
      </Text>
    </View>
  );
};

const App: () => Node = () => {
  const isDarkMode = useColorScheme() === 'dark';

  const backgroundStyle = {
    backgroundColor: isDarkMode ? Colors.darker : Colors.lighter,
  };

  const [count, setCount] = useState(0);
  const [isOpen, setIsOpen] = useState(false);
  const [option, setOption] = useState("");
  return (
    <SafeAreaView style={backgroundStyle}>
      <StatusBar barStyle={isDarkMode ? 'light-content' : 'dark-content'} />
      <ScrollView
        contentInsetAdjustmentBehavior="automatic"
        style={backgroundStyle}>
        <Header />
        <View
          style={{
            backgroundColor: isDarkMode ? Colors.black : Colors.white,
          }}>
              <BlankUserControl  onHappened={(arg) => alert(JSON.stringify(arg.nativeEvent)) } />
                  <Button content={{ string: `Last selected option = ${option} ${count}` }} foreground="red"
                      onClick={(a) => { alert(JSON.stringify(a.nativeEvent)); setCount(count + 1); setIsOpen(true); }} />
                  {/*<StackPanel orientation="horizontal">*/}
                  {/*    <HyperlinkButton content={{ string: "Click me!" }} onClick={(args) => {*/}
                  {/*        alert(`clicked! Native event args: ${JSON.stringify(args.nativeEvent)}`);*/}
                  {/*    }} />*/}
                  {/*    <Border verticalAlignment="center" background="paleturquoise" >*/}
                  {/*        <TextBlock text="this is a textblock" foreground='red' textAlignment="center" />*/}
                  {/*    </Border>*/}
                  {/*    <TextBlock text="this is another textblock" foreground='green' textAlignment="center" />*/}
                  {/*    <Button content={{ string: "this is a button" }} onClick={() => { alert("you clicked the button!"); }} />*/}
                  {/*</StackPanel>*/}
                  {/*<ComboBox text="this is a combobox" description={{ string: "best bois" }} onSelectionChanged={(args) => {*/}
                  {/*    alert(`sel changed! Native event args: ${JSON.stringify(args.nativeEvent)}`);*/}
                  {/*}} >*/}
                  {/*    <ComboBoxItem content={{ string: "garfield" }} foreground="black" />*/}
                  {/*    <ComboBoxItem content={{ string: "snoopy" }} foreground="black" />*/}
                  {/*</ComboBox>*/}
                  <TextBox text="this is a textbox with a menuFlyout" foreground="red">
                      <NativeXamlControl type="menuFlyout" isOpen={isOpen} onClosed={() => {
                          setIsOpen(false);
                      }} >
                          <MenuFlyoutItem text="option 1" onClick={(x) => { alert(JSON.stringify(x.nativeEvent)); setOption("option 1"); }} />
                          <MenuFlyoutItem text="option 2" onClick={() => { alert("clicked 2"); setOption("option 2"); }}/>
                    </NativeXamlControl>
                  </TextBox>

                  {/*<TextBox text="this is a textbox with a flyout" */}
                  {/*  foreground="red" >*/}
                  {/*  <NativeXamlControl type="flyout" >*/}
                  {/*    <Button content={{string: "click me"}} />*/}
                  {/*  </NativeXamlControl>*/}
                  {/*</TextBox>*/}

          <Section title="Step One">
            Edit <Text style={styles.highlight}>App.js</Text> to change this
            screen and then come back to see your edits.
          </Section>
          <Section title="See Your Changes">
            <ReloadInstructions />
          </Section>
          <Section title="Debug">
            <DebugInstructions />
          </Section>
          <Section title="Learn More">
            Read the docs to discover what to do next:
          </Section>
          <LearnMoreLinks />
        </View>
      </ScrollView>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  sectionContainer: {
    marginTop: 32,
    paddingHorizontal: 24,
  },
  sectionTitle: {
    fontSize: 24,
    fontWeight: '600',
  },
  sectionDescription: {
    marginTop: 8,
    fontSize: 18,
    fontWeight: '400',
  },
  highlight: {
    fontWeight: '700',
  },
});

export default App;
