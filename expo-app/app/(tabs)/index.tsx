import { View } from 'react-native';
import WebView from 'react-native-webview';

export default function HomeScreen() {
  return (
    <View style={{ flex: 1 }}>
      <WebView 
        source={{ uri: 'http://100.66.150.158:3000/' }} 
        style={{ flex: 1 }}
      />
    </View>
  );
}