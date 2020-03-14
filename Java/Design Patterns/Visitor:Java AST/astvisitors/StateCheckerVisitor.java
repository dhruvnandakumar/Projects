package ecs160.visitor.astvisitors;

import ecs160.visitor.utilities.UtilReader;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.dom.*;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class StateCheckerVisitor extends ASTVisitor {
    private String contextClass;
    private String absName;
    private List<String> contextMethods;
    private List<String> abstractMethods;
    private List<String> methodsMatched;
    private int rightCalls;
    private String stateVarName;
    private boolean contNow;
    private String contextLookingAt;


    private StateCheckerVisitor(String c, String a) {
        contextClass = c;
        contextMethods = new ArrayList<>();
        abstractMethods = new ArrayList<>();
        methodsMatched = new ArrayList<>();
        rightCalls = 0;
        stateVarName = "";
        absName = a;
        contNow = false;
        contextLookingAt = "";
    }

    public static StateCheckerVisitor setUpGrader(String contextPath, String contextName, String abstractPath, String abstractName) {

        File contextFile = new File(contextPath);
        String text = "";
        try {
            text = UtilReader.read(contextFile);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        ASTParser parser1 = ASTParser.newParser(AST.JLS12); //Create a parser for a version of the Java language (12 here)
        Map<String, String> options1 = JavaCore.getOptions(); //get the options for a type of Eclipse plugin that is the basis of Java plugins
        options1.put(JavaCore.COMPILER_SOURCE, JavaCore.VERSION_12); //Specify that we are on Java 12 and add it to the options...
        parser1.setCompilerOptions(options1); //forward all these options to our parser
        parser1.setKind(ASTParser.K_COMPILATION_UNIT); //What kind of constructions will be parsed by this parser.  K_COMPILATION_UNIT means we are parsing whole files.
        parser1.setResolveBindings(true); //Enable looking for bindings/connections from this file to other parts of the program.
        parser1.setBindingsRecovery(true); //Also attempt to recover incomplete bindings (only can be set to true if above line is set to true).
        String[] classpath1 = {System.getProperty("java.home") + "/lib/rt.jar"}; //Link to your Java installation.
        parser1.setEnvironment(classpath1, new String[]{""}, new String[]{"UTF-8"}, true);
        parser1.setSource(text.toCharArray()); //Load in the text of the file to parse.
        parser1.setUnitName(contextFile.getAbsolutePath()); //Load in the absolute path of the file to parse
        CompilationUnit contextCompUnit = (CompilationUnit) parser1.createAST(null); //Create the tree and link to the root node.


        File file = new File(abstractPath);
        String text1 = "";
        try {
            text1 = UtilReader.read(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        ASTParser parser = ASTParser.newParser(AST.JLS12); //Create a parser for a version of the Java language (12 here)
        Map<String, String> options = JavaCore.getOptions(); //get the options for a type of Eclipse plugin that is the basis of Java plugins
        options.put(JavaCore.COMPILER_SOURCE, JavaCore.VERSION_12); //Specify that we are on Java 12 and add it to the options...
        parser.setCompilerOptions(options); //forward all these options to our parser
        parser.setKind(ASTParser.K_COMPILATION_UNIT); //What kind of constructions will be parsed by this parser.  K_COMPILATION_UNIT means we are parsing whole files.
        parser.setResolveBindings(true); //Enable looking for bindings/connections from this file to other parts of the program.
        parser.setBindingsRecovery(true); //Also attempt to recover incomplete bindings (only can be set to true if above line is set to true).
        String[] classpath = {System.getProperty("java.home") + "/lib/rt.jar"}; //Link to your Java installation.
        parser.setEnvironment(classpath, new String[]{""}, new String[]{"UTF-8"}, true);
        parser.setSource(text1.toCharArray()); //Load in the text of the file to parse.
        parser.setUnitName(file.getAbsolutePath()); //Load in the absolute path of the file to parse
        CompilationUnit abstractCompUnit = (CompilationUnit) parser.createAST(null); //Create the tree and link to the root node.

        StateCheckerVisitor stateChecker = new StateCheckerVisitor(contextName, abstractName);

        abstractCompUnit.accept(stateChecker);

        contextCompUnit.accept(stateChecker);

        return stateChecker;

    }

    public boolean visit(TypeDeclaration node) {

        String actualName = node.getName().getIdentifier();

        if (actualName.equals(this.contextClass)) {
            this.contNow = true;
        }
        else
            this.contNow = false;

        if(actualName.equals(contextClass) || actualName.equals(absName))
            return true;

        return false;
    }

    public boolean visit(FieldDeclaration node) {


        String fieldName = "N/A";

        if (node.getType().isSimpleType()) {
            if (((SimpleType) node.getType()).getName().isSimpleName()) {
                fieldName = ((SimpleName) ((SimpleType) node.getType()).getName()).getIdentifier();
            }
        }


        List<VariableDeclarationFragment> frags = node.fragments();

        if(fieldName.equals(absName)){
            if(frags.size() == 1){
                for(VariableDeclarationFragment f : frags)
                    stateVarName = f.toString();
            }
        }

        return false;

    }

    public boolean visit(MethodDeclaration node) {

        boolean returnValue = false;
        String methodName = node.getName().getIdentifier();

        if (contNow)
            contextMethods.add(methodName);
        else
            abstractMethods.add(methodName);

        //Abstract class already parsed if we are looking at context class
        if(contNow){
            if(abstractMethods.indexOf(methodName) != -1){
                contextLookingAt = methodName;
                returnValue = true;
            }

        }

        return returnValue;
    }

    public boolean visit(MethodInvocation node) {
        String methodName = node.getName().getIdentifier();

        if (methodName.equals(contextLookingAt)) {

            boolean correctCaller = false;

            if(node.getExpression().toString().equals(stateVarName)){
                correctCaller = true;
            }

            if(correctCaller && methodsMatched.indexOf(methodName) == -1){
                rightCalls++;
                methodsMatched.add(methodName);
            }
        }

        return true;
    }

    public boolean gradeA() {

        boolean allPresent = true;
        int idx;

        for (String cMethod : abstractMethods) {
            idx = contextMethods.indexOf(cMethod);
            if (idx == -1)
                allPresent = false;
        }

        return allPresent;
    }

    public int gradeB() {
        return rightCalls;
    }

}
